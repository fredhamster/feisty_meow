<?php
namespace App\View\Cell;

use Cake\View\Cell;
use Cake\Log\Log;
use DateTime;
use Google_Service_Calendar;


/**
 * GoogleCalendar cell
 */
class GoogleCalendarCell extends Cell
{

    /**
     * List of valid options that can be passed into this
     * cell's constructor.
     *
     * @var array
     */
    protected $_validCellOptions = [];

    /*
     * returns an array of calendar events based on the google client and
     * the range between the start and end times.
     * 
     * if the access token has died or suffered some unfortunate accident, then
     * null is returned and authorization must be reattempted.
     */
    public static function prepareCalendar($client, $startTime = null, $endTime = null)
    {    	
    	$cal = new Google_Service_Calendar ( $client );
    	Log::debug ( 'created google calendar object' );
    	
    	$calendarId = 'primary';
    	
    	if (! $startTime) {
    		// default is to start with today, but not with a time.
    		$startTime = new DateTime();
    		$startTime->setTime(0, 0);
    	}
    	
    	if (! $endTime ) {
    		$endTime = new DateTime($startTime->format('c'));
    		$endTime->modify ( "+31 day" ); // month from now (ish).
    	}
    	
    	Log::debug ( 'start: ' . $startTime->format ( 'c' )  . ' end: ' . $endTime->format('c'));
    	    	
    	$optParams = array (
    			// 'maxResults' => 100,
    			'orderBy' => 'startTime',
    			'singleEvents' => TRUE,
    			'timeMin' => $startTime->format('c'),
    			'timeMax' => $endTime->format ( 'c' )
    	);
    	
    	try {
    		$results = $cal->events->listEvents ( $calendarId, $optParams );
    	} catch (\Exception $e) {
    		Log::debug('caught an exception from listing events!');
    		return null;
    	}
    	
    	Log::debug ( 'after listEvents call.' );
    	
    	// the calEvents array will be an associative array where the keys are the
    	// dates that events start.  the value stored for each key is in turn an associative array of
    	// good stuff about the event.  the good stuff includes 'info', for the appointment description,
    	// 'start' for the starting date (plus time perhaps), and 'event' for the raw google event
    	// contents.
    	$calEvents = [ ];
    	
    	if (count ( $results->getItems () ) == 0) {
    		// nothing to add to array.
    	} else {
    		foreach ( $results->getItems () as $event ) {
    			$info = $event->getSummary ();
    			
    			$start = $event->start->dateTime;
    			if (empty ( $start )) {
    				// simple index by date.
    				$index = $event->start->date;
    				$start = 'All Day';  // our time mutates.
    			} else {
    				// make the index from just the date portion.
    				$bits = str_split($start, strpos($start, 'T'));
    				$index = $bits[0];
    				
    				// clean up the time portion for presentation.
    				$date = new DateTime($start);
    				$start = $date->format('H:i:s');
    				
    				//Log::debug('chopped time of start is: ' . $start);
    			}
    			
    			$existing_entry = [];
    			if (array_key_exists($index, $calEvents)) {
    				$existing_entry = $calEvents[$index];
    			}
    			
    			array_push ( $existing_entry, [
    					'info' => $info,
    					'start' => $start,
    					'event' => $event,
    			] );
    			
    			$calEvents[$index] = $existing_entry;
    		}
    	}
    	
    	return $calEvents;
    }
    
    /**
     * Default display method.
     *
     * needs to be passed a GoogleClient object that has already been configured with a valid
     * access token.
     *
     * @return void
     */
    public function display($calEvents)
    {
    	
    	$this->set ( 'calEvents', $calEvents );
    }
}
