<?php
namespace App\Model\Table;

use Cake\Log\Log;
use Cake\ORM\Query;
use Cake\ORM\RulesChecker;
use Cake\ORM\Table;
use Cake\Validation\Validator;

/**
 * Locations Model
 *
 * @method \App\Model\Entity\Location get($primaryKey, $options = [])
 * @method \App\Model\Entity\Location newEntity($data = null, array $options = [])
 * @method \App\Model\Entity\Location[] newEntities(array $data, array $options = [])
 * @method \App\Model\Entity\Location|bool save(\Cake\Datasource\EntityInterface $entity, $options = [])
 * @method \App\Model\Entity\Location patchEntity(\Cake\Datasource\EntityInterface $entity, array $data, array $options = [])
 * @method \App\Model\Entity\Location[] patchEntities($entities, array $data, array $options = [])
 * @method \App\Model\Entity\Location findOrCreate($search, callable $callback = null, $options = [])
 *
 * @mixin \Cake\ORM\Behavior\TimestampBehavior
 */
class LocationsTable extends Table
{

    /**
     * Initialize method
     *
     * @param array $config The configuration for the Table.
     * @return void
     */
    public function initialize(array $config)
    {
        parent::initialize($config);

        $this->setTable('locations');
        $this->setDisplayField('name');
        $this->setPrimaryKey('id');

        $this->addBehavior('Timestamp');
        
        $this->belongsToMany('Categories',
        		[
        				'targetForeignKey' => 'category_id',
        				'foreignKey' => 'location_id',
        				'joinTable' => 'categories_locations',
        				'through' => 'CategoriesLocations',
        		]);
        
        
    }

    /**
     * Default validation rules.
     *
     * @param \Cake\Validation\Validator $validator Validator instance.
     * @return \Cake\Validation\Validator
     */
    public function validationDefault(Validator $validator)
    {
        $validator
            ->integer('id')
            ->allowEmpty('id', 'create');

        $validator
            ->requirePresence('name', 'create')
            ->notEmpty('name')
            ->add('name', 'unique', ['rule' => 'validateUnique', 'provider' => 'table']);

        $validator->requirePresence ( 'location', 'create' )->notEmpty ( 'location' );
		
		$validator
            ->allowEmpty('lat');
        $validator
            ->allowEmpty('lng');
            
        return $validator;
    }

    /**
     * Returns a rules checker object that will be used for validating
     * application integrity.
     *
     * @param \Cake\ORM\RulesChecker $rules The rules object to be modified.
     * @return \Cake\ORM\RulesChecker
     */
    public function buildRules(RulesChecker $rules)
    {
        $rules->add($rules->isUnique(['name']));

        return $rules;
    }
    
    
    /**
     * returns a query that will find the category records associated with a location 'id'
     * as a list.
     */
    public function getSelectedCategories($id)
    {
    	// find the categories that are chosen for this item, if any.
    	$selectedCategories = $this->CategoriesLocations->find ( 'list', [
    			'keyField' => 'category_id',
    			'valueField' => 'location_id',
    			'conditions' => [
    					'location_id' => $id
    			],
    			'contain' => [
    					'Categories'
    			]
    	] );    	
    	return $selectedCategories;
    }
	
	/**
	 * returns a query that will find all of the locations within the bounding box.
	 */
	public function getLocationsInBox($sw_lat, $sw_long, $ne_lat, $ne_long, $start = null, $end = null) {
		$locationsInRange = $this->find ( 'all', [ 
			'contain' => ['Categories'],
		] );
		
		Log::Debug ( 'bounds=' . $sw_lat . ', ' . $sw_long . ' to ' . $ne_lat . ', ' . $ne_long );
		
		$bounds = [ 
				$sw_lat,
				$sw_long,
				$ne_lat,
				$ne_long 
		];
		
		if (! $bounds) {
			Log::Debug ( "failed to calculate the bounding box!" );
		} else {
			Log::Debug ( "bounding box: " . var_export ( $bounds, true ) );
		}
		
		// use the boundaries to restrict the lookup so we aren't crushed.
		// order: min_lat, min_long, max_lat, max_long.
		$locationsInRange = $locationsInRange->where ( function ($exp) use ($bounds) {
			return $exp->gte ( 'lat', $bounds [0] )->gte ( 'lng', $bounds [1] )->lte ( 'lat', $bounds [2] )->lte ( 'lng', $bounds [3] );
		} );
		
		if (($start !== null) && ($end !== null)) {
			Log::debug('start of range = ' . $start . ' and end = ' . $end);
			$locationsInRange = $locationsInRange->order(['lat desc', 'lng desc']);
			$chunk = $end - $start + 1;
			$locationsInRange = $locationsInRange->limit($chunk);
			$page = 1 + (int)($start / $chunk);
			$locationsInRange = $locationsInRange->page($page);
			Log::debug('page = ' . $page . ' and chunk = ' . $chunk);
		}
		
		return $locationsInRange;
	}
	
	/**
	 * retrieves all of the locations within the bounding box as a list of partial location data.
	 */
	public function getChewedLocationsInBox($sw_lat, $sw_long, $ne_lat, $ne_long, $start = null, $end = null) {
		$locationsInRange = $this->getLocationsInBox($sw_lat, $sw_long, $ne_lat, $ne_long, $start, $end);
		
		// track locations we've already added and do not add ones at exactly the same lat/long.
		// if we did add these, google maps screws up in representing them.
		$locationsToSerialize = [ ];
		
		foreach ( $locationsInRange as $location ) {
			$lat_and_long = $location->lat . ',' . $location->lng;
			if (array_key_exists ( $lat_and_long, $locationsToSerialize )) {
				continue; // already got it.
			}

// Log::debug('dumping location row: ' . var_export($location, true));
			
			// we don't include lat and lng below since they are encoded as array key.
			$locationsToSerialize [$lat_and_long] = [ 
					'name' => $location->name,
					'loc' => $location->location,
					'id' => $location->id 
			];
			
			// only try to add the image if the location actually has a category membership (at least one).
			if ($location->categories) {
				$locationsToSerialize [$lat_and_long]['icon'] = $location->categories[0]->image;
			}						
		}
		
    	return $locationsToSerialize;
    }
    
}
