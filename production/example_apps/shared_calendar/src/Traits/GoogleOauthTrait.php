<?php
namespace App\Traits;

use Cake\Log\Log;
use Google_Client;

/**
 * A trait that provides google oauth authorization helper methods. 
 */
trait GoogleOauthTrait
{
    // constants used for items stored in the session.
    public function POST_AUTHORIZATION_JUMP() { return 'postAuthJump'; }
    public function STORED_OAUTH_TOKEN() { return 'lastOauthToken'; }
    public function OAUTH_SCOPES_REQUESTED() { return 'oauthScopes'; }
    
    /**
     * sets the next place in our app to go after authorization.  the url needs to be a link
     * which can consume the access token after it's available (from getLastOauthToken() below).
     */
    public function setPostAuthorizationURL($url)
    {
     	if (session_status() == PHP_SESSION_NONE) {
     		Log::debug('starting session for set post auth url');
    		session_start();
     	}
    	$_SESSION [$this->POST_AUTHORIZATION_JUMP()] = $url;
    }
    
    /**
     * retrieves any post authorization url that was registered.
     * this destroys the item in the session also, so it is available for a single use.
     */
    public function getPostAuthorizationURL()
    {
     	if (session_status() == PHP_SESSION_NONE) {
     		Log::debug('starting session for get post auth url');
    		session_start();
     	}
     	if (isset ( $_SESSION [$this->POST_AUTHORIZATION_JUMP()] ) && $_SESSION [$this->POST_AUTHORIZATION_JUMP()]) {
     		$url = $_SESSION [$this->POST_AUTHORIZATION_JUMP()];
    		Log::debug('loaded url for post auth as: ' . $url);
    		unset($_SESSION[$this->POST_AUTHORIZATION_JUMP()]);
    		return $url;
    	} else {
    		Log::debug('could not find post authorization url in session!');
    		
    		return null;
    	}
    }

    /**
     * before we can request oauth authorization, we need to know the scopes that will be used
     * by our application.  this allows them to be set in the session, where scopes should be an
     * array of valid scope names (defined by the google oauth API).
     */
    public function setRequestedScopes($scopes)
    {
    	if (session_status() == PHP_SESSION_NONE) {
    		Log::debug('starting session for set req scopes');
    		session_start();
    	}
    	$_SESSION [$this->OAUTH_SCOPES_REQUESTED()] = $scopes;
    }
    
    /**
     * gets the array of scopes out of the session for use by the next oauth call.
     * this will NOT destroy the scopes; that must be done manually with dropRequestedScopes
     * below, since we need this session item multiple times.
     */
    public function getRequestedScopes()
    {
    	if (session_status() == PHP_SESSION_NONE) {
    		Log::debug('starting session for get req scopes');    		
    		session_start();
    	}
    	if (isset ( $_SESSION [$this->OAUTH_SCOPES_REQUESTED()] ) && $_SESSION [$this->OAUTH_SCOPES_REQUESTED()]) {
    		$scopes = $_SESSION [$this->OAUTH_SCOPES_REQUESTED()];
    		return $scopes;
    	} else {
    		return null;
    	}
    }
    
    /**
     * throws the requested scopes out of the session.
     */
    public function dropRequestedScopes()
    {
    	if (session_status() == PHP_SESSION_NONE) {
    		Log::debug('starting session for drop req scopes');
    		session_start();
    	}
    	unset($_SESSION[$this->OAUTH_SCOPES_REQUESTED()]);
    }
    
    
    /**
     * saves the result of oauth authorization, an access token, into the session.
     */
    public function setLastOAuthToken($token)
    {
    	if (session_status() == PHP_SESSION_NONE) {
    		Log::debug('starting session for set last oauth token');    		
    		session_start();
    	}
    	$_SESSION [$this->STORED_OAUTH_TOKEN()] = $token;
    }
    
    /**
     * retrieves the last stored oauth token.  this destroys the item in the session
     * afterwards, so you get one chance for retrieval of the token.
     */
    public function getLastOAuthToken()
    {
    	if (session_status() == PHP_SESSION_NONE) {
    		Log::debug('starting session for set last oauth token');    		
    		session_start();
    	}
    	if (isset ( $_SESSION [$this->STORED_OAUTH_TOKEN()] ) && $_SESSION [$this->STORED_OAUTH_TOKEN()]) {
    		$token = $_SESSION [$this->STORED_OAUTH_TOKEN()];
    		unset($_SESSION[$this->STORED_OAUTH_TOKEN()]);
    		return $token;
    	} else {
    		return null;
    	}
    }
    
    /**
     * sets up and configures the Google client object using a 'client_secret.json' file
     * stored in the config directory.  A scope name or an array of scopes should be passed
     * in the "scopes" parameter.  if the "redirectUri" parameter is not null, then this is set
     * as the singular redirection URI (which is needed if the client secret has several redirect
     * URIs listed).
     */
    public function createGoogleClient($scopes, $redirectUri = null) {
    	$client = new Google_Client ();
    	
    	// see https://developers.google.com/api-client-library/php/auth/web-app for info on creating the secret.
    	$client->setAuthConfig ( 'config/client_secret.json' );
    	
    	// use the redirect link if they gave us one.
    	if ($redirectUri) {
    		$client->setRedirectUri ( $redirectUri );
    	}
    	
    	// register for offline access, so we don't need user to be logged in.
    	$client->setAccessType ( "offline" );
    	// enable incremental authorization.
    	$client->setIncludeGrantedScopes ( true );
    	
    	// add the scope(s) we're interested in here.
    	$client->addScope ( $scopes );
    	
    	return $client;
    }
    
    /**
     * *deprecated* creates a google client configured by discrete parameters.
     *
     * note: does not free one to use just any redirect url for login; that's set at credential creation time.
     */
    public function createGoogleClientUsingParameters($clientId, $clientSecret, $applicationName, $redirectUri, $scopes) {
    	$client = new Google_Client ();
    	
    	// see https://developers.google.com/api-client-library/php/auth/web-app for info on creating the secret.
    	$client->setClientId ( $clientId );
    	$client->setClientSecret ( $clientSecret );
    	$client->setApplicationName ( $applicationName );
    	$client->setRedirectUri ( $redirectUri );
    	
    	// register for offline access, so we don't need user to be logged in.
    	$client->setAccessType ( "offline" );
    	// enable incremental authorization.
    	$client->setIncludeGrantedScopes ( true );
    	
    	// add the scope we're interested in here.
    	$client->addScope ( $scopes);
    	
    	return $client;
    }
    
    /**
     * checks whether the google client object's token is still valid.  if not, the token is refreshed.
     * this will only work with tokens that were originally requested for offline access and that possess
     * the refresh token.
     * if the refresh was done, the newly revitalized token is returned and must be stored.
     * if the token is already okay, then null is returned (which avoids tricky comparisons to
     * determine if an update happened).
     */
    public function freshenTokenInClient($client) 
    {
    	if (! $client->isAccessTokenExpired ()) {
    		// no refresh needed.
    		return null;
    	}
    	// currently assuming the new access token also contains the old refresh token.
    	// this has been borne out by results from google.
    	Log::debug ( 'noticed that the access token has expired!' );
    	
    	// retrieve a new access token using our refresh token.
    	$refresher = $client->getRefreshToken ();
    	$client->fetchAccessTokenWithRefreshToken ( $refresher );
    	$accessToken = $client->getAccessToken ();
    	
    	//bad! do not show this in real app.
    	//Log::debug ( 'got a new access token after refresh: ' . var_export ( $accessToken, true ) );
    	
    	// return the new and tasty token.
    	return $accessToken;
    }
    
    /**
     * throws out the specified access token, which means that the app will have to reauthorize
     * to get another token.
     */
    public function revokeToken($token)
    {
    	$client = new Google_Client ();
    	// apparently the client needs very little setup to perform this if we have the whole token?
    	$client->revokeToken ( $token );
    }
}

/*
 * hmmm: still need a method for doing additive authorizations, where we request new scopes and
 * have them bundled onto our existing access token.  would need to store the token afterwards also.
 */


