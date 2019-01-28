<?php
namespace App\Controller;

use App\Controller\AppController;
use Avmaps\Controller\Component\SimpleMapsComponent;
use Cake\Log\Log;

/**
 * Categories Controller
 *
 * @property \App\Model\Table\CategoriesTable $Categories
 */
class CategoriesController extends AppController
{	
	// keeps track of the API key to be used for our google queries, if one is known.
	private $api_key = null;
	
	public function initialize()
	{
		parent::initialize();
		
		// load up the request handler to make processing easier on ajax requests.
		$this->loadComponent('RequestHandler');		
		
		// configure the request handler to deal with json encoding.
 		$this->RequestHandler->config('inputTypeMap.json', ['json_decode', true]);		
				
		$this->api_key = SimpleMapsComponent::getGoogleAPIKey();
		
		// allow simple access to location db.
		$this->loadModel ( 'Locations' );
		
	}
	


    /**
     * Index method
     *
     * @return \Cake\Network\Response|null
     */
    public function index()
    {
        $categories = $this->paginate($this->Categories);

        $this->set(compact('categories'));
        $this->set('_serialize', ['categories']);
    }

    /**
     * View method
     *
     * @param string|null $id Category id.
     * @return \Cake\Network\Response|null
     * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
     */
    public function view($id = null)
    {
        $category = $this->Categories->get($id, [
        		'contain' => ['Locations']
        ]);

        $this->set('category', $category);
        $this->set('_serialize', ['category']);
        
        // query all the locations belonging to this category but only retrieve part of the info.
    	$locationBits = $this->Categories->CategoriesLocations->find('list', [
    			'conditions' => [ 'category_id' => $id ],
    			'contain' => ['Locations'],
    			'valueField' => function ($row) {
    				return $row->location->get('lat')
    				 . ',' . $row->location->get('lng')
    				. ' ' . $row->location->get('location');
    			}
    	]);
    	$this->set('locationBits', $locationBits);
    }
    
    /**
     * queries the locations to display on a map given an "id" to lookup in categories table.
     */
    public function map($id = null)
    {
    	$category = $this->Categories->get($id, [
    			'contain' => ['Locations']
    	]);
    	
    	$locationsFound = $this->Categories->getLocationsInCategory($id);
    	$this->set('locationsInCategory', $locationsFound);    	

    	// grab the flags in requests.
    	$parms = $this->request->getQueryParams();
    	
    	// we will cluster by default, but they can also pass the clustering flag with 0 or 1.
    	$clustering = (array_key_exists('clustering', $parms) && ($parms['clustering'] == '1'))
    		|| !array_key_exists('clustering', $parms);
    	//Log::Debug('clustering is: ' . var_export($clustering, true));
    	
    	// set up the selected items in our config chooser.
    	$selectedList = [];    	
    	if ($clustering) {
    		array_push($selectedList, 'clustering');
    	}
    	$this->set('selectedList', $selectedList);
    	    	
    	// pass the pre-loading function in for the map element.
    	$this->set('dbProcessor', $this);
    	$this->set('preloader', '$this->MapDisplay->addMarkers($dbProcessor, $locationsInCategory, $category );');    	
    	
    	if ($this->request->is(['post', 'put'])) {
    			//, 'patch'])) {
    		$postData = $this->request->getData('view_options')['_ids'];
    		
    		$new_clustering = is_array($postData) && in_array('clustering', $postData);
    		
    		if ($clustering != $new_clustering) {
    			$this->redirect(['action' => 'map', 
    					'0' => $id,
    					'?' => [
    							'clustering' => $new_clustering,
    					]
    			]);
    		}
    	}
    	
    	$this->set('category', $category);
    	    	
//     	$this->set('_serialize', ['category']);
    }
    
    /**
     * processes a location row by extracting the important information into an array.
     * separates the db structure from the pieces needed for google maps.
     */
    public function processRow(& $location_row)
    {
    	return [
	    	'lat' => $location_row ['location'] ['lat'],
	    	'lng' => $location_row ['location'] ['lng'],
	    	'title' => $location_row ['location'] ['name'],
	    	// kludge below adds extra space on content, since someone is not left justifying these.
	    	'content' => h ( $location_row ['location'] ['location'] ) . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;',
  			];
    }
    
    public function extractCategoryImage(& $category)
    {
    	return $category->image;
    }
    
    
    /**
     * attempts to jump to the center of the category, given a particular interpretation of the
     * category.  if possible we will decide that the category is a state name, and then attempt
     * to jump to the center of that state.
     * will also show markers in the category.
     */
    public function center($id = null)
    {
    	$category = $this->Categories->get($id, [
    			'contain' => ['Locations']
    	]);
    	
    	// grab the flags in requests.
    	$parms = $this->request->getQueryParams();
    	
    	// we will cluster by default, but they can also pass the clustering flag with 0 or 1.
    	$clustering = (array_key_exists('clustering', $parms) && ($parms['clustering'] == '1'))
    		|| !array_key_exists('clustering', $parms);
    	//Log::Debug('clustering is: ' . var_export($clustering, true));
    	
    	// set up the selected items in our config chooser.
    	$selectedList = [];
    	if ($clustering) {
    		array_push($selectedList, 'clustering');
    	}
    	$this->set('selectedList', $selectedList);
    	
    	$state = null;
    	// interpret the category name as a state if possible.
    	if (substr($category->name, 0, 3) == 'US-') {
    		// bingo, we have a US prefix on the string, so decide what state they mean.
    		$state = substr($category->name, 3, 2);
    		
    	}
    	// perform the lookup to get the readable name.  otherwise a state like indiana (IN) will
    	// not get geocoded properly.  probably others as well.
    	$state = SimpleMapsComponent::lookupStateFromAbbreviation($state);
    	
    	$this->set('state', $state);
    	
    	Log::debug('state was found as: ' . $state);

    	if ($state !== null) {
			$centeredPoint = SimpleMapsComponent::geocode($state . " USA", [
				'key' => $this->api_key
			]);
			$mapCenter = [ 'lat' => $centeredPoint[0], 'lng' => $centeredPoint[1] ] ;
			$this->set('mapCenter', $mapCenter);
			Log::Debug('computed a map center of: ' . var_export($mapCenter, true));
    	}   
    	
    	
    	// thoughts on algorithm: is the center of the state according to google what we want to rely on?
    	// we could at least start there, record the lat/longs and then edit them later as we decide.    	

    	$radius = null;
    	$lat = null;
    	$long = null;
    	if ($mapCenter) {
    		$radius = 40; // miles from center to show markers.
    		$lat = $mapCenter['lat'];
    		$long = $mapCenter['lng'];
    	}    	
    	
    	Log::Debug('given lat=' . $lat . ' long='.  $long . ' radius=' . $radius);
    	
    	Log::Debug('loading locations in category within radius');
    	
    	// compute the lat/long bounding box for our search.
    	$bounds = SimpleMapsComponent::calculateLatLongBoundingBox ( $lat, $long, $radius );
    	
    	if (! $bounds) {
    		Log::Debug("failed to calculate the bounding box!");
    	} else {
    		Log::Debug("bounding box: " . var_export($bounds, true));
    	}
    	
    	// query all the locations belonging to this category within the box.
    	$locationsInCategory = $this->Categories->getLocationsInCategoryInBox($id,
    			$bounds [0], $bounds [1], $bounds [2], $bounds [3] );
    	
    	$this->set('locationsInCategory', $locationsInCategory);
    	
    	$this->set('dbProcessor', $this);    	
    	$this->set('preloader', '$this->MapDisplay->addMarkers($dbProcessor, $locationsInCategory, $category );');
    	
    	if ($this->request->is(['post', 'put'])) {
    		//, 'patch'])) {
    		$postData = $this->request->getData('view_options')['_ids'];
    		
    		$new_clustering = is_array($postData) && in_array('clustering', $postData);
    		
    		if ($clustering != $new_clustering) {
    			$this->redirect(['action' => 'map',
    					'0' => $id,
    					'?' => [
    							'clustering' => $new_clustering,
    					]
    			]);
    		}
    	}
    	
    	$this->set('category', $category);
//     	$this->set('_serialize', ['category','selectedList']);
    }
    
    /**
     * Add method
     *
     * @return \Cake\Network\Response|null Redirects on successful add, renders view otherwise.
     */
    public function add()
    {
        $category = $this->Categories->newEntity();
        if ($this->request->is('post')) {
            $category = $this->Categories->patchEntity($category, $this->request->getData());
            if ($this->Categories->save($category)) {
                $this->Flash->success(__('The category has been saved.'));

                return $this->redirect(['action' => 'index']);
            }
            $this->Flash->error(__('The category could not be saved. Please, try again.'));
        }
        $this->set(compact('category'));
        $this->set('_serialize', ['category']);
    }

    /**
     * Edit method
     *
     * @param string|null $id Category id.
     * @return \Cake\Network\Response|null Redirects on successful edit, renders view otherwise.
     * @throws \Cake\Network\Exception\NotFoundException When record not found.
     */
    public function edit($id = null)
    {
        $category = $this->Categories->get($id, [
            'contain' => []
        ]);
        if ($this->request->is(['patch', 'post', 'put'])) {
            $category = $this->Categories->patchEntity($category, $this->request->getData());
            if ($this->Categories->save($category)) {
                $this->Flash->success(__('The category has been saved.'));

                return $this->redirect(['action' => 'index']);
            }
            $this->Flash->error(__('The category could not be saved. Please, try again.'));
        }
        $this->set(compact('category'));
        $this->set('_serialize', ['category']);
    }

    /**
     * Delete method
     *
     * @param string|null $id Category id.
     * @return \Cake\Network\Response|null Redirects to index.
     * @throws \Cake\Datasource\Exception\RecordNotFoundException When record not found.
     */
    public function delete($id = null)
    {
        $this->request->allowMethod(['post', 'delete']);
        $category = $this->Categories->get($id);
        if ($this->Categories->delete($category)) {
            $this->Flash->success(__('The category has been deleted.'));
        } else {
            $this->Flash->error(__('The category could not be deleted. Please, try again.'));
        }

        return $this->redirect(['action' => 'index']);
    }
    
    /**
     * provides ajax friendly responses.  expects to be passed 'category' in post data.
     * requires bounding box for lookup be passed as 'sw_lat', 'sw_lng', 'ne_lat', 'ne_lng'. 
     */
    public function lookupajax()
    {
    	$reqData = $this->request->getData();
    	
    	Log::Debug('got to lookup with data: ' . var_export($reqData, true));

    	// check that this is a post method, since we don't support anything else.
    	if (! $this->request->is(['post'])) {
    		die('this is a post method');
    	}

    	if (array_key_exists('action', $reqData)) {
    		$action = $reqData['action'];
    		
    		if (strcasecmp($action, 'lookupBox') == 0) {
    			$this->findLocationsInCategoryWithinBounds($reqData);
    		} else if (strcasecmp($action, 'getInfo') == 0) {
    			$this->getInfoOnLocation($reqData);
    		} else {
    			die('lookupajax call was given unknown action: ' . $action);    			
    		}
    		
    	} else {
    		die('lookupajax call has no action specified');
    	}
    }
    
    public function findLocationsInCategoryWithinBounds($reqData)
    {	
    	if (array_key_exists('category', $reqData)) {
    		$id = $reqData['category'];
    	} else {
    		// throw an exception here?
    		$message = 'failed to find category id in request data';
    		Log::Debug($message);
    		die($message);
    	}
    	
    	$category = $this->Categories->get($id, [
    			'contain' => ['Locations']
    	]);
    	$this->set('category', $category);
    	
    	if (array_key_exists('sw_lat', $reqData)) {
    		$sw_lat = $reqData['sw_lat'];
    	}
    	if (array_key_exists('sw_lng', $reqData)) {
    		$sw_lng = $reqData['sw_lng'];
    	}
    	if (array_key_exists('ne_lat', $reqData)) {
    		$ne_lat = $reqData['ne_lat'];
    	}
    	if (array_key_exists('ne_lng', $reqData)) {
    		$ne_lng = $reqData['ne_lng'];
    	}
    	
    	//temp!  fails over to using whole range.
    	if ($sw_lat === null) { $sw_lat = -90; }
    	if ($sw_lng === null) { $sw_lng = -180; }
    	if ($ne_lat === null) { $ne_lat = 90; }
    	if ($ne_lng === null) { $ne_lng = 180; }

    	$start = null;
    	if (array_key_exists('start', $reqData)) {
    		$start = $reqData['start'];
    	}
    	$end = null;
    	if (array_key_exists('end', $reqData)) {
    		$end = $reqData['end'];
    	}
    	
    	// lookup the locations inside that box and store for view.
    	$locationsToSerialize = $this->Categories->getChewedLocationsInCategoryInBox($id, $sw_lat, $sw_lng, $ne_lat, $ne_lng, $start, $end);    	
    	//Log::debug('before encoding, php array looks like: ' . var_export($locationsToSerialize, true));    	
    	Log::Debug("returning json now...");    
    	$encoded = json_encode($locationsToSerialize);
    	//     	Log::debug('encoded json is: ' . var_export($encoded, true));
    	die($encoded);    	
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
