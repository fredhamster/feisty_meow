<?php
namespace App\Controller;

use App\Controller\AppController;
use Cake\Log\Log;
use App\Traits\GoogleOauthTrait;

/**
 * Authorizer Controller
 * 
 * This controller provides an entry point for authorization processes.
 * So far this is just for Google's OAuth 2.0.
 */
class AuthorizerController extends AppController
{
	use GoogleOauthTrait;
	
	/*
	 * Routing note:
	 * 
	 * This controller will provide a link for /authorizer/google_login where the oauth
	 * process can come back to our application from google.  To make your client secret setup
	 * simpler, add a route at the top level like so:
	 * 	 $routes->connect('/google_oauth', [ 'controller'=>'authorizer', 'action' => 'google_login']);
	 */
	
	public function initialize()
	{
		parent::initialize();
	}
	
	/**
	 * our callback from google oauth that is passed the oauth access token (or an error
	 * if authorization failed).
	 * before redirecting to this URL, one must use the GoogleOauth trait's
	 * setPostAuthorizationURL() and setRequestedScopes() methods to provide session
	 * parameters (since this link is invoked by google later, and they will not be
	 * providing any of this info).
	 */
	public function googleLogin() {
		if (session_status() == PHP_SESSION_NONE) {
			session_start ();
		}
		
		// retrieve the scopes out of the session.
		$scopes = $this->getRequestedScopes();
		Log::debug('loaded scopes: ' . var_export($scopes, true));
		
		// use the scopes in a new google client.
		$client = $this->createGoogleClient ( $scopes );		
		
		// see if we already have the 'code' available from the google side.
		if (! isset ( $_GET ['code'] )) {
			// no code, so we need to jump over to google.
			Log::Debug ( 'creating auth url to redirect to google oauth' );
			$auth_url = $client->createAuthUrl ();
			$this->redirect ( $auth_url );
		} else {						
			// we've got our code, so now we can try to fetch our access token.
			Log::Debug ( 'access token being actively acquired...' );
			$client->fetchAccessTokenWithAuthCode ( $_GET ['code'] );
			// clean out the scopes in session now that we're done with them.
			$this->dropRequestedScopes();
			
			// record the new token in our session.
			$token = $client->getAccessToken ();
			$this->setLastOAuthToken($token);
			
			// fabulously bad idea to show this...
			//Log::debug ( 'got access token: ' . var_export ( $token, true ) );			
						
			// go to the next point in our app where we can handle the newly stored token.
			$redirect =  $this->getPostAuthorizationURL();
			if (! $redirect) {
				// jump home if they registered no continuation.  this is a serious error in flow.
				$redirect = 'http://' . $_SERVER ['HTTP_HOST'] . '/';
				Log::debug('failure to find the redirection location for our app after successful oauth');
			}
			$this->redirect ( $redirect );
		}
	}
	
}
