<?php

namespace App\Controller;

use App\Controller\AppController;
use Avmaps\Controller\Component\SimpleMapsComponent;
use Cake\Log\Log;

/**
 * Locations Controller
 *
 * @property \App\Model\Table\LocationsTable $Locations
 */
class LocationsController extends AppController {
	
	// keeps track of the API key to be used for our google queries, if one is known.
	private $api_key = null;
	
	/**
	 * initializer method.
	 */
	public function initialize() {
		parent::initialize ();
		
		$this->loadComponent ( 'Avmaps.SimpleMaps' );
		$this->loadModel ( 'Categories' );
		
		$this->api_key = SimpleMapsComponent::getGoogleAPIKey ();
	}
	
	/**
	 * Index method
	 *
	 * @return \Cake\Network\Response|null
	 */
	public function index() {
		$locations = $this->paginate ( $this->Locations, [ 
				'contain' => 'Categories' 
		] );
		$this->set ( compact ( 'locations' ) );
		$this->set ( '_serialize', [ 
				'locations' 
		] );
	}
	
	/**
	 * sets two variables for the view: 'categoriesList' with *all* the category names that exist and
	 * 'selectedList' with the categories associated with the location 'id'.
	 *
	 * @param int $id        	
	 */
	public function loadAssociatedCategories($id) {
		// find all of the categories available.
		$this->set ( 'categoriesList', $this->Categories->getAllCategories());
				
		// turn the chosen categories into a list of category ids for the multi-select.
		$selectedCategories = $this->Locations->getSelectedCategories($id);
		$selectedList = array_keys ( $selectedCategories->toArray () );
		$this->set ( 'selectedList', $selectedList );
	}
	
	/**
	 * calculates the set of locations within a certain range from a starting point and returns the
	 * full set.
	 */
	public function loadLocationsInRange($lat, $long, $radius) {
		Log::debug ( 'into ranged locations calculator' );
		
		// compute the lat/long bounding box for our search.
		$bounds = SimpleMapsComponent::calculateLatLongBoundingBox ( $lat, $long, $radius );
		
		if (! $bounds) {
			Log::debug ( "failed to calculate the bounding box!" );
		} else {
			Log::debug ( "bounding box: " . var_export ( $bounds, true ) );
		}
		
		// use the boundaries to restrict the lookup so we aren't crushed.
		// order: min_lat, min_long, max_lat, max_long.
		$locationsInRange = $this->Locations->getLocationsInBox($bounds [0], $bounds [1], $bounds [2], $bounds [3]);
		
		// heavy!
		// Log::debug('got a list of locations: ' + var_export($locationsInRange->toArray(), true));
		
		$this->set ( 'locationsInRange', $locationsInRange );
	}
	
	/**
	 * View method
	 *
	 * @param string|null $id
	 *        	Location id.
	 * @return \Cake\Network\Response|null
	 * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
	 */
	public function view($id = null) {
		$location = $this->Locations->get ( $id, [ 
				'contain' => [ 
						'Categories' 
				] 
		] );
		
		$this->loadAssociatedCategories ( $id );
		
		$this->set ( 'api_key', $this->api_key );
		
		$this->set ( 'location', $location );
		$this->set ( '_serialize', [ 
				'location' 
		] );
	}
	
	/**
	 * Add method
	 *
	 * @return \Cake\Network\Response|null Redirects on successful add, renders view otherwise.
	 */
	public function add() {
		$location = $this->Locations->newEntity ();
		
		$categoriesList = $this->Categories->find ( 'list', [ 
				'keyField' => 'id',
				'valueField' => 'name' 
		] );
		$this->set ( 'categoriesList', $categoriesList );
		
		if ($this->request->is ( 'post' )) {
			$location = $this->Locations->patchEntity ( $location, $this->request->getData () );
			
			Log::debug ("patching with " .  var_export($location, true) );
			
			$location = $this->SimpleMaps->fillInGeoPosition ( $location, [ 
					'key' => $this->api_key 
			] );
			
			if ($location !== false && $this->Locations->save ( $location )) {
				$this->Flash->success ( __ ( 'The location has been saved.' ) );
				
				return $this->redirect ( [ 
						'action' => 'index' 
				] );
			}
			$this->Flash->error ( __ ( 'The location could not be saved. Please, try again.' ) );
		}
		
		$this->set ( compact ( 'location' ) );
		$this->set ( '_serialize', [ 
				'location' 
		] );
	}
	
	/**
	 * Edit method
	 *
	 * @param string|null $id
	 *        	Location id.
	 * @return \Cake\Network\Response|null Redirects on successful edit, renders view otherwise.
	 * @throws \Cake\Network\Exception\NotFoundException When record not found.
	 */
	public function edit($id = null) {
		$location = $this->Locations->get ( $id, [ 
				'contain' => [ 
						'Categories' 
				] 
		] );
		
		$this->loadAssociatedCategories ( $id );
		
		if ($this->request->is ( [ 
				'patch',
				'post',
				'put' 
		] )) {
			$location = $this->Locations->patchEntity ( $location, $this->request->getData () );
			
			$new_location = $this->SimpleMaps->fillInGeoPosition ( $location, [ 
					'key' => $this->api_key 
			] );
			if ($new_location === false) {
				$this->Flash->error ( __ ( 'The location could not be geocoded. Please, try again.' ) );
			} else {
				$location = $new_location;
				if ($this->Locations->save ( $location )) {
					$this->Flash->success ( __ ( 'The location has been saved.' ) );
					return $this->redirect ( [ 
							'action' => 'index' 
					] );
				}
				$this->Flash->error ( __ ( 'The location could not be saved. Please, try again.' ) );
			}
		}
		$this->set ( compact ( 'location' ) );
		$this->set ( '_serialize', [ 
				'location' 
		] );
	}
	
	/**
	 * Delete method
	 *
	 * @param string|null $id
	 *        	Location id.
	 * @return \Cake\Network\Response|null Redirects to index.
	 * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
	 */
	public function delete($id = null) {
		$this->request->allowMethod ( [ 
				'post',
				'delete' 
		] );
		$location = $this->Locations->get ( $id );
		if ($this->Locations->delete ( $location )) {
			$this->Flash->success ( __ ( 'The location has been deleted.' ) );
		} else {
			$this->Flash->error ( __ ( 'The location could not be deleted. Please, try again.' ) );
		}
		
		return $this->redirect ( [ 
				'action' => 'index' 
		] );
	}
	
	
	// global locations list, loaded once per object creation.
	private $locationsListGlobal = null;
	
	/**
	 * generates a random list of locations with a limited number of items.
	 */
	public function grabLocationsList() 
	{
		if ($this->locationsListGlobal)
			return $this->locationsListGlobal;
		
		/*
		 * load up a list of randomly chosen locations for the selection lists. we will keep this
		 * around if possible, rather than reloading per page view.
		 */
		$this->locationsListGlobal = $this->Locations->find ( 'list', [ 
				'keyField' => 'id',
				'valueField' => 'name' 
		] )->limit ( 1000 )->order ( 'rand()' )->toArray ();
		
		// $Log::debug('got a result array: ' . var_export($this->locationsListGlobal, true));
		
		return $this->locationsListGlobal;
	}
	
	/**
	 * adds an item to the location list to ensure a user will not see their previous choice disappear.
	 */
	public function addLocationToHeldList($id, $entry) {
		$this->locationsListGlobal [$id1] = $entry;
	}
	
	
	/**
	 * calculate the distance between two locations in the db.
	 * will allow picking if one or both
	 * location ids are missing.
	 */
	public function distance($id1 = null, $id2 = null) {
		// pretty kludgy approach here; don't yet know how to make the form refresh
		// without reloading it, but reloading it clears the selections. so we're redirecting
		// the form to itself but with the id parameters filled in.
		
		// process the parameters, if any were provided.
		$this->set ( 'fromId', $id1 );
		$this->set ( 'toId', $id2 );
		
		// load the actual location info if they specified the ids already.
		if ($id1 !== null) {
			$this->set ( 'fromAddress', $this->Locations->get ( $id1 ) ['location'] );
			$fromGeoCoord = $this->Locations->get ( $id1 ) ['lat'] . ',' . $this->Locations->get ( $id1 ) ['lng'];
			// ensure it's in our global list also, or it won't get selected.
			$this->addLocationToHeldList($id1, $this->Locations->get ( $id1 ) ['name']);
		} else {
			$this->set ( 'fromAddress', null );
		}
		if ($id2 !== null) {
			$this->set ( 'toAddress', $this->Locations->get ( $id2 ) ['location'] );
			$toGeoCoord = $this->Locations->get ( $id2 ) ['lat'] . ',' . $this->Locations->get ( $id2 ) ['lng'];
			;
			// add to our global list for selection.
			$this->addLocationToHeldList($id2, $this->Locations->get ( $id2 ) ['name']);
		} else {
			$this->set ( 'toAddress', null );
		}
		
		if ($id1 === null || $id2 === null) {
			// set default value for distance.
			$distance = 'unknown';
		} else {
			// calculate distance between locations.
			$distance = $this->SimpleMaps->calculateDrivingDistance ( $fromGeoCoord, $toGeoCoord, [ 
					'key' => $this->api_key 
			] );
			// $this->Flash->log ( 'distance calculated is ' . $distance );
			if ($distance === false) {
				// failed to calculate this, so we let the user know.
				$distance = "Unable to calculate a route using Google Maps Distance Matrix";
			}
		}
		// store in distance calculated variable.
		$this->set ( 'distanceCalculated', $distance );
		
		// load up the selection lists for from and to addresses.
		$this->set ( 'locationsFrom', $this->grabLocationsList());
		$this->set ( 'locationsTo', $this->grabLocationsList() );
		
		if ($this->request->is ( 'post' )) {
			$datapack = $this->request->getData ();
			
			$fromId = $datapack ['from'] ['_ids'];
			$this->Flash->log ( h ( 'from id is ' . $fromId ) );
			$toId = $datapack ['to'] ['_ids'];
			$this->Flash->log ( h ( 'to id is ' . $toId ) );
			
			$fromGeoCoord = $this->Locations->get ( $fromId ) ['lat'] . ',' . $this->Locations->get ( $fromId ) ['lng'];
			
			$this->Flash->log ( 'from coord is ' . $fromGeoCoord );
			$toGeoCoord = $this->Locations->get ( $toId ) ['lat'] . ',' . $this->Locations->get ( $toId ) ['lng'];
			;
			$this->Flash->log ( 'to coord is ' . $toGeoCoord );
			
			// how to make the form show the same data but with updated distance?
			// currently kludged...
			return $this->redirect ( [ 
					'action' => 'distance',
					$fromId,
					$toId 
			] );
		}
	}
	
	/**
	 * finds all the locations within a given radius (in miles) from the location with 'id'.
	 */
	public function radius($id = null, $radius = 20) {
		Log::debug ( 'into the radius method in controller...' );
		
		$this->set ( 'id', $id );
		if ($id != null) {
			$this->set ( 'fromAddress', $this->Locations->get ( $id ) ['location'] );
			$fromLat = $this->Locations->get ( $id ) ['lat'];
			$this->set ( 'fromLat', $fromLat );
			$fromLong = $this->Locations->get ( $id ) ['lng'];
			$this->set ( 'fromLong', $fromLong );
		}
		
		if ($this->request->is ( 'post' )) {
			
			$datapack = $this->request->getData ();
			
			// look for approximate array index.
			Log::debug ( 'got datapack: ' . var_export ( $datapack, true ) );
			foreach ( $datapack as $key => $value ) {
				if ("radius" == substr ( $key, 0, 6 )) {
					$radius = $value;
				}
			}
		}
		
		$this->set ( 'radius', $radius );
		
		$this->set('dbProcessor', $this);
		$this->set('preloader', '$this->MapDisplay->addMarkers($dbProcessor, $locationsInRange, null, $radius, $fromLat . \',\'. $fromLong);');
		
		$this->loadLocationsInRange ( $fromLat, $fromLong, $radius );
	}
	
//hmmm: the below should be listed in an interface.
	/**
	 * processes a location row by extracting the important information into an array.
	 * separates the db structure from the pieces needed for google maps.
	 */
	public function processRow(& $location_row)
	{		
		return [
				'lat' => $location_row  ['lat'],
				'lng' => $location_row ['lng'],
				'title' => $location_row ['name'],
				// kludge below adds extra space on content, since someone is not left justifying these.
				'content' => h ( $location_row ['location'] ) . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;',
		];
	}
	
	public function extractCategoryImage(& $category)
	{
		return $category->image;
	}
	
	
	/**
	 * jumps to a particular location as the center of the map and shows locations nearby.
	 */
	public function jump($id) {
		Log::debug ( 'into the jump method in locations controller...' );
		
		$this->set ( 'id', $id );
		if ($id != null) {
			$fromLat = $this->Locations->get ( $id ) ['lat'];
			$this->set ( 'fromLat', $fromLat );
			$fromLong = $this->Locations->get ( $id ) ['lng'];
			$this->set ( 'fromLong', $fromLong );
		}
		
		$locationsInRange = $this->Locations->find ( 'all', [ 
				'conditions' => [ 
						'id' => $id 
				] 
		] );
		
		// we were handed a list of locations that match our query, so we can now add them as markers.
		$this->set('dbProcessor', $this);		
		$this->set('preloader', '$this->MapDisplay->addMarkers($dbProcessor, $locationsInRange, null, 1000, $fromLat . \',\'. $fromLong);');
		
		
		// heavy!
		// Log::debug('got a list of locations: ' + var_export($locationsInRange->toArray(), true));
		
		$this->set ( 'locationsInRange', $locationsInRange );
	}
	
	/**
	 * provides restful api for looking up locations within two lat/long boundaries.
	 */
	public function lookupajax() {
		Log::debug ( 'into lookupajax method in locations controller...' );
		
		$reqData = $this->request->getData ();
		
		Log::Debug ( 'got to locations lookup with data: ' . var_export ( $reqData, true ) );
		
		// check that this is a post method, since we don't support anything else.
		if (! $this->request->is ( [ 
				'post' 
		] )) {
			die ( 'this is a post method' );
		}
		
		
		if (array_key_exists('action', $reqData)) {
			$action = $reqData['action'];
			
			if (strcasecmp($action, 'lookupBox') == 0) {
				$this->findLocationsWithinBounds($reqData);
			} else if (strcasecmp($action, 'getInfo') == 0) {
				$this->getInfoOnLocation($reqData);
			} else {
				die('lookupajax call was given unknown action: ' . $action);
			}
			
		} else {
			die('lookupajax call has no action specified');
		}
	}
	
	public function findLocationsWithinBounds($reqData)
	{		
		if (array_key_exists ( 'sw_lat', $reqData )) {
			$sw_lat = $reqData ['sw_lat'];
		}
		if (array_key_exists ( 'sw_lng', $reqData )) {
			$sw_lng = $reqData ['sw_lng'];
		}
		if (array_key_exists ( 'ne_lat', $reqData )) {
			$ne_lat = $reqData ['ne_lat'];
		}
		if (array_key_exists ( 'ne_lng', $reqData )) {
			$ne_lng = $reqData ['ne_lng'];
		}

		if (array_key_exists ( 'radius', $reqData )) {
			$radius = $reqData ['radius'];
		}
		
		$start = null;
		if (array_key_exists('start', $reqData)) {
			$start = $reqData['start']; 
		}
		$end = null;
		if (array_key_exists('end', $reqData)) {
			$end = $reqData['end'];
		}
		
		// temp! fails over to using whole range.
		if ($sw_lat === null) {
			$sw_lat = - 90;
		}
		if ($sw_lng === null) {
			$sw_lng = - 180;
		}
		if ($ne_lat === null) {
			$ne_lat = 90;
		}
		if ($ne_lng === null) {
			$ne_lng = 180;
		}
		
		// lookup the locations inside that box and store for view.
		$locationsToSerialize = $this->Locations->getChewedLocationsInBox($sw_lat, $sw_lng, $ne_lat, $ne_lng, $start, $end);
		Log::debug('db found ' . sizeof($locationsToSerialize) . ' rows for query (' . $start . '-' . $end . ')');

		// simple implementation here since cakephp v3.4 was doing weird stuff instead of returning object we chose to serialize.
		// ajax method would consistently return the name of the variable and 'undefined' as the only value, rather than properly
		// serializing.
		$encoded = json_encode ( $locationsToSerialize );
		// Log::debug('encoded json is: ' . var_export($encoded, true));
		
		die ( $encoded );
	}
	
	public function getInfoOnLocation($reqData)
	{
		if (array_key_exists('id', $reqData)) {
			$id = $reqData['id'];
		} else {
			// throw an exception here?
			$message = 'failed to find location id in request data';
			Log::Debug($message);
			die($message);
		}
		
		$location = $this->Locations->get($id, [
		]);
		return die(json_encode($location));
	}
	
}
