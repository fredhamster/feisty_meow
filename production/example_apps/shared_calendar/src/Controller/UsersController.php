<?php

namespace App\Controller;

use App\Controller\AppController;
use Cake\Core\Configure;
use Cake\Cache\Cache;
use Cake\Log\Log;
use DateTime;
use Google_Service_Calendar;
use App\Traits\GoogleOauthTrait;
use App\View\Cell\GoogleCalendarCell;

/**
 * Users Controller
 *
 * @property \App\Model\Table\UsersTable $Users
 *
 * @method \App\Model\Entity\User[] paginate($object = null, array $settings = [])
 */
class UsersController extends AppController 
{
	use GoogleOauthTrait;
	
	public function initialize()
	{
		parent::initialize();
		
		// configure the cache for our class.
		
		$prefix = 'av_'. strtolower($this->name) .'-';
		// av_ prefix so we can delete all files generated in cache/sql/ except for .gitignore
		
		$locales = Configure::read('avenger.locales');		
		if(!empty($locales)) {			
			$prefix .= Configure::read('Config.language') .'-';			
		}
		Log::debug('cache prefix will be: ' . $prefix);
		
		// configure a file cache for storing calendar data.
		Cache::config('calendar_cache', [
				'className' => 'File',
				'prefix'     => $prefix,
				'duration'   => '+2 hours',			
		]);		
	}
	
	/**
	 * Index method
	 *
	 * @return \Cake\Http\Response|void
	 */
	public function index() {
		$users = $this->paginate ( $this->Users );
		
		$this->set ( compact ( 'users' ) );
		$this->set ( '_serialize', [ 
				'users' 
		] );
	}
	
	/**
	 * View method
	 *
	 * @param string|null $id
	 *        	User id.
	 * @return \Cake\Http\Response|void
	 * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
	 */
	public function view($id = null) {
		$user = $this->Users->get ( $id, [ 
				'contain' => [ ] 
		] );
		
		$this->set ( 'user', $user );
		$this->set ( '_serialize', [ 
				'user' 
		] );
	}
	
	/**
	 * prepares the google oauth process by stuffing required information into the session.
	 * we'll then redirect over to the authorizer controller to get the work done.
	 */
	public function requestGoogleOauth($id)
	{
		//hmmm: if no id, bail out to error page.

		Log::debug("into requestGoogleOauth...");
		
		if (session_status() == PHP_SESSION_NONE) {
			session_start ();
		}
		
		// add the post-authorization redirect url for the auth controller.
		$this->setPostAuthorizationURL('/users/callback/' . $id);
		// set the scopes that we want to request.
		$this->setRequestedScopes([Google_Service_Calendar::CALENDAR_READONLY]);
		
// 		//temp
// 		$urlcheck = $_SESSION[$this->POST_AUTHORIZATION_JUMP()];
// 		Log::debug('callback url found in session: ' . var_export($urlcheck, true));
// 		$scopecheck = $_SESSION[$this->POST_AUTHORIZATION_JUMP()];
// 		Log::debug('scopes found in session: ' . var_export($scopecheck , true));
		
		Log::debug("before redirecting from requestGoogleOauth...");
				
		// get the user to authorize us, and subsequently we'll record the access token.
		$this->redirect ( [
				'controller' => 'Authorizer',
				'action' => 'googleLogin'
		] );
	}
	
	/**
	 * Add method
	 *
	 * @return \Cake\Http\Response|null Redirects on successful add, renders view otherwise.
	 */
	public function add() {
		session_start ();
		
		$user = $this->Users->newEntity ();
		if ($this->request->is ( 'post' )) {
			$user = $this->Users->patchEntity ( $user, $this->request->getData () );
			if ($this->Users->save ( $user )) {
				$this->Flash->success ( __ ( 'The user has been saved.' ) );
				// now get the oauth mojo working...
				$this->requestGoogleOauth($user->id);
				return;
			}
			$this->Flash->error ( __ ( 'The user could not be saved. Please, try again.' ) );
		}
		$this->set ( compact ( 'user' ) );
		$this->set ( '_serialize', [ 
				'user' 
		] );
	}
	
	/**
	 * a whole bunch of redirection happens when the oauth process starts, but this is our
	 * return point for this application after all that's done.  this is registered as the
	 * post authorization redirection location, and should have a valid access token available
	 * for the specified user.
	 * 
	 * @param unknown $id
	 */
	public function callback($id) {
		Log::debug ( 'got to the users controller callback' );
		
		$user = $this->Users->get ( $id );
		
		// retrieve the access token, which is hopefully totally valid now...
		$token = $this->getLastOAuthToken ();
		if ($token) {
			$user->token = json_encode ( $token );
			$this->Users->save ( $user );
		}
		$this->set ( 'user', $user );
		$this->set ( '_serialize', [ 
				'user' 
		] );
	}
	
	/**
	 * Edit method
	 *
	 * @param string|null $id
	 *        	User id.
	 * @return \Cake\Http\Response|null Redirects on successful edit, renders view otherwise.
	 * @throws \Cake\Network\Exception\NotFoundException When record not found.
	 */
	public function edit($id = null) {
		$user = $this->Users->get ( $id, [ 
				'contain' => [ ] 
		] );
		if ($this->request->is ( [ 
				'patch',
				'post',
				'put' 
		] )) {
			$user = $this->Users->patchEntity ( $user, $this->request->getData () );
			if ($this->Users->save ( $user )) {
				$this->Flash->success ( __ ( 'The user has been saved.' ) );
				
				return $this->redirect ( [ 
						'action' => 'index' 
				] );
			}
			$this->Flash->error ( __ ( 'The user could not be saved. Please, try again.' ) );
		}
		$this->set ( compact ( 'user' ) );
		$this->set ( '_serialize', [ 
				'user' 
		] );
	}
	
	/**
	 * Delete method
	 *
	 * @param string|null $id
	 *        	User id.
	 * @return \Cake\Http\Response|null Redirects to index.
	 * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
	 */
	public function delete($id = null) {
		$this->request->allowMethod ( [ 
				'post',
				'delete' 
		] );
		$user = $this->Users->get ( $id );
		if ($this->Users->delete ( $user )) {
			$this->Flash->success ( __ ( 'The user has been deleted.' ) );
		} else {
			$this->Flash->error ( __ ( 'The user could not be deleted. Please, try again.' ) );
		}
		
		return $this->redirect ( [ 
				'action' => 'index' 
		] );
	}
	
	/**
	 * shows the upcoming appointments on the calendar.
	 */
	public function showCalendar($id) {
		Log::debug ( 'into the show calendar method' );
		
		session_start ();
		
		$user = $this->Users->get ( $id );

		if (! $user->token) {
			// apparently we need to re-acquire this for the user.
			// NOTE: in real app, this will be a failure and a place to send email.
			$this->requestGoogleOauth($id);
			return;
		}
		$accessToken = json_decode ( $user->token, true );
		
		$client = $this->createGoogleClient( [ 
				Google_Service_Calendar::CALENDAR_READONLY 
		] );
		
		$client->setAccessToken ( $accessToken );
		
		// make sure that the token is still valid, and if not, refresh it.
		$newToken = $this->freshenTokenInClient($client);
		if ($newToken) {
			// save the new token in the db again.
			$accessToken = $newToken;
			$user->token = json_encode ( $accessToken );
			$this->Users->save ( $user );
		}

		// start from beginning of today.
		$start = new DateTime();
		$start->setTime(0, 0);
		
		// see if we have a cached calendar available.
		$cacheKey = 'cal-' . $id . '--' . $start->format('Y-m-d_H-i-s');
		$calEvents = Cache::read($cacheKey, 'calendar_cache');
		if ($calEvents) {
			Log::debug('found cached events for calendar.');
		} else {
			// an unfortunate kludge below; if we delay calling the google listEvents method,
			// we can get an exception that requires reauthorization.  but the view cell has no
			// way to get to the controller to start the authorization process again, so we do
			// it here instead.
			$calEvents = GoogleCalendarCell::prepareCalendar($client, $start);
			Log::debug('contacted google to get events for calendar.');
			if (! $calEvents) {
				// we have lost our authorization somehow.
				$this->requestGoogleOauth($id);
				return;
			}
			Cache::write($cacheKey, $calEvents, 'calendar_cache');
		}
		$this->set('calEvents', $calEvents);
		$this->set ( '_serialize', [
				'calEvents'
		] );
	}
	
	/**
	 * revokes authorization granted by google for current token.
	 */
	public function revoke($id) {
		$user = $this->Users->get ( $id );
		
		$this->set ( 'user', $user );
		$this->set ( '_serialize', [ 
				'user' 
		] );
		
		$token = json_decode ( $user->token, true );
		
		Log::debug ( 'revoking token: ' . var_export ( $token, true ) );

		$this->revokeToken($token);		
		Log::debug ( 'after revoking the access token.' );
		
		// update with no token.
		$user->token = null;
		$this->Users->save ( $user );
	}
	

}
