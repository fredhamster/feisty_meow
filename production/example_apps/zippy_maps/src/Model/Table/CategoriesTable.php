<?php
namespace App\Model\Table;

use Cake\Log\Log;
use Cake\ORM\Query;
use Cake\ORM\RulesChecker;
use Cake\ORM\Table;
use Cake\Validation\Validator;

/**
 * Categories Model
 *
 * @method \App\Model\Entity\Category get($primaryKey, $options = [])
 * @method \App\Model\Entity\Category newEntity($data = null, array $options = [])
 * @method \App\Model\Entity\Category[] newEntities(array $data, array $options = [])
 * @method \App\Model\Entity\Category|bool save(\Cake\Datasource\EntityInterface $entity, $options = [])
 * @method \App\Model\Entity\Category patchEntity(\Cake\Datasource\EntityInterface $entity, array $data, array $options = [])
 * @method \App\Model\Entity\Category[] patchEntities($entities, array $data, array $options = [])
 * @method \App\Model\Entity\Category findOrCreate($search, callable $callback = null, $options = [])
 *
 * @mixin \Cake\ORM\Behavior\TimestampBehavior
 */
class CategoriesTable extends Table
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

        $this->setTable('categories');
        $this->setDisplayField('name');
        $this->setPrimaryKey('id');

        $this->addBehavior('Timestamp');
        
        $this->belongsToMany('Locations',
        		[
        				'targetForeignKey' => 'location_id',
        				'foreignKey' => 'category_id',
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

        $validator
            ->integer('parent')
            ->allowEmpty('parent');

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
     * returns a query that will find all the categories as a list.
     */
    public function getAllCategories() {
    	// find the full list of categories to show the user.
    	$categoriesList = $this->find ( 'list', [
    			'keyField' => 'id',
    			'valueField' => 'name',
    			'order' => [
    					'name' => 'asc'
    			]
    	] );
		return $categoriesList;
    }
    
    /**
     * returns a query that will locate all of the locations in the specified category with 'id'.
     */
    public function getLocationsInCategory($id)
    {
    	// query all the locations belonging to this category.
    	$locationsInCategory = $this->CategoriesLocations->find('all', [
    			'conditions' => [ 'category_id' => $id ],
    			'contain' => ['Locations']
    	]);
    	return $locationsInCategory;
    }
    
    /**
     * returns a query that finds all locations in the category that fit within the southwest and northeast
     * corners specified.
     */
    public function getLocationsInCategoryInBox($id, $sw_lat, $sw_long, $ne_lat, $ne_long, $start = null, $end = null)
    {
    	// query all the locations belonging to this category.
    	$locationsInCategory = $this->CategoriesLocations->find('all', [
    			'conditions' => [ 'category_id' => $id ],
    			'contain' => ['Locations', 'Categories']
    	]);
    	
    	Log::Debug('bounds=' . $sw_lat . ', '.  $sw_long . ' to ' . $ne_lat . ', ' . $ne_long);
    	
    	$bounds = [ $sw_lat , $sw_long , $ne_lat , $ne_long ];
    	
    	if (! $bounds) {
    		Log::Debug("failed to calculate the bounding box!");
    	} else {
    		Log::Debug("bounding box: " . var_export($bounds, true));
    	}
    	
    	// use the boundaries to restrict the lookup.
    	// order: min_lat, min_long, max_lat, max_long.
    	$locationsInCategory = $locationsInCategory->where (
    			function ($exp) use ($bounds) {
    				return $exp->gte ( 'lat', $bounds [0] )->gte ( 'lng', $bounds [1] )->lte ( 'lat', $bounds [2] )->lte ( 'lng', $bounds [3] );
    			}
    			);
    	
    	if (($start !== null) && ($end !== null)) {
    		Log::debug('start of range = ' . $start . ' and end = ' . $end);
    		$locationsInCategory= $locationsInCategory->order(['lat desc', 'lng desc']);
    		$chunk = $end - $start + 1;
    		$locationsInCategory= $locationsInCategory->limit($chunk);
    		$page = 1 + (int)($start / $chunk);
    		$locationsInCategory= $locationsInCategory->page($page);
    		Log::debug('page = ' . $page . ' and chunk = ' . $chunk);
    	}    	
    	
    	return $locationsInCategory;
    }
    
    public function getChewedLocationsInCategoryInBox($id, $sw_lat, $sw_long, $ne_lat, $ne_long, $start = null, $end = null)
    {
    	$locationsInCategory = $this->getLocationsInCategoryInBox($id, $sw_lat, $sw_long, $ne_lat, $ne_long, $start, $end);
    	
    	$locationsToSerialize = [];
    	
    	// make an array with the useful parts of the location data.
    	foreach ($locationsInCategory as $location) {    		
    		$lat_and_long = $location->location->lat. ',' . $location->location->lng;
    		if (array_key_exists ( $lat_and_long, $locationsToSerialize)) {
    			continue;  // already got it.
    		}
    		
//Log::debug('got an icon value of: ' . $location->category->image);    		
    		
    		// we don't include lat and lng below since they are encoded as array key.
    		$locationsToSerialize[$lat_and_long] = ['name' => $location->location->name,
    				'loc' => $location->location->location,
    				'icon' => $location->category->image,
    				'id' => $location->location_id,
    		];    		
    	}
    	
    	return $locationsToSerialize;
    }
}
