<?php
 /*******************************************************************************
 *                ZIP Code and Distance Claculation Class
 *******************************************************************************
 *      Author:     Micah Carrick
 *      Email:      email@micahcarrick.com
 *      Website:    http://www.micahcarrick.com
 *
 *      File:       zipcode.class.php
 *      Version:    1.2.0
 *      Copyright:  (c) 2005 - Micah Carrick 
 *                  You are free to use, distribute, and modify this software 
 *                  under the terms of the GNU General Public License.  See the
 *                  included license.txt file.
 *
 *******************************************************************************
 *  VERION HISTORY:
 *      v1.2.0 [Oct 22, 2006] - Using a completely new database based on user
                                contributions which resolves many data bugs.
                              - Added sorting to get_zips_in_range()
                              - Added ability to include/exclude the base zip
                                from get_zips_in_range()
                              
 *      v1.1.0 [Apr 30, 2005] - Added Jeff Bearer's code to make it MUCH faster!
 
 *      v1.0.1 [Apr 22, 2005] - Fixed a typo :)
 
 *      v1.0.0 [Apr 12, 2005] - Initial Version
 *
 *******************************************************************************
 *  DESCRIPTION:
 
 *    A PHP Class and MySQL table to find the distance between zip codes and 
 *    find all zip codes within a given mileage or kilometer range.
 *      
 *******************************************************************************
 *
 * Note:  The following 2 database definitions are needed to import zipcode data from
 * the new style P.O. data which is a great departure.
 * One will either have to dynamically specify the data source or
 * create zipcode_import.php and zipcode_new.php models containing below information
 * 
 *
 *class ZipcodeNew extends AppModel {
   var $name = 'ZipcodeNew';
   var $useTable = 'zip_code_new';
   var $primaryKey = 'id';
   var $useDbConfig = 'avengerShared';
}

class ZipcodeImport extends AppModel {
   var $name = 'ZipcodeImport';
   var $useTable = 'free_zipcode_database';
   var $primaryKey = 'RecordNumber';
   var $useDbConfig = 'avengerShared';

}

 *
 *
 *
 *
*/

// constants for setting the $units data member
define('_UNIT_MILES', 'm');
define('_UNIT_KILOMETERS', 'k');

// constants for passing $sort to get_zips_in_range()
define('_ZIPS_SORT_BY_DISTANCE_ASC', 1);
define('_ZIPS_SORT_BY_DISTANCE_DESC', 2);
define('_ZIPS_SORT_BY_ZIP_ASC', 3);
define('_ZIPS_SORT_BY_ZIP_DESC', 4);

// constant for miles to kilometers conversion
define('_M2KM_FACTOR', 1.609344);


class Zipcode extends AppModel {
   var $name = 'Zipcode';
   var $useTable = 'zip_code';
   var $primaryKey = 'zip_code';
   var $useDbConfig = 'avengerShared';
   var $zipInRangeData = array();
   var $zipPageConditions = array();

   
	function paginate($conditions, $fields, $order, $limit, $page = 1, $recursive = null) {
		//$conditions[] ="1 = 1 GROUP BY week, away_team_id, home_team_id";
		//$recursive = -1;
		//$fields = array('week', 'away_team_id', 'home_team_id');
		$this->get_zips_in_range($this->zipPageConditions['zip'], $this->zipPageConditions['range'], $this->zipPageConditions['sort']);
		$page_data = array();
		//loop through page data
		
		
		return $this->zipInRangeData;
	}
	
	function paginateCount($conditions = null, $recursive = 0) {
		return count($this->zipInRangeData);
	}
	
   
   
   /*
    * TAKEN FROM MICCAH CODE
    */
   
	var $last_error = "";            // last error message set by this class
   var $units = _UNIT_MILES;        // miles or kilometers
   var $decimals = 2;               // decimal places for returned distance
   
   function get_distance($zip1, $zip2) {

      // returns the distance between to zip codes.  If there is an error, the 
      // function will return false and set the $last_error variable.
      
      if ($zip1 == $zip2)
      	return 0; // same zip code means 0 miles between. :)
   
   
      // get details from database about each zip and exit if there is an error
      
      $details1 = $this->get_zip_point($zip1);
      $details2 = $this->get_zip_point($zip2);
      if ($details1 == false) {
         $this->last_error = "No details found for zip code: $zip1";
         return false;
      }
      if ($details2 == false) {
         $this->last_error = "No details found for zip code: $zip2";
         return false;
      }     


      // calculate the distance between the two points based on the lattitude
      // and longitude pulled out of the database.
      
      $miles = $this->calculate_mileage($details1['lat'], $details2['lat'], $details1['lon'], $details2['lon']);
      
      
 
      if ($this->units == _UNIT_KILOMETERS) return round($miles * _M2KM_FACTOR, $this->decimals);
      else return round($miles, $this->decimals);       // must be miles
      
   }   

   
	/**
	 * 
	 * Lookups up a zip code and returns single array or multi-array based on sending multiple zip codes in array or just a string
	 * @param $zip
	 */
	function lookup($zip, $options = array()) {
		$default_options = 
			array(
				'bare' => false, 	//set to true to strip outer Zipcode dimension
			);
		$options = am($default_options, $options);
		
		if(is_array($zip)) {
			// find all
			$data = $this->find('all', array('conditions'=>array('Zipcode.zip_code'=>$zip), 'recursive' => -1));
		} else {
			// find first
			$data = $this->find('first', array('conditions'=>array('Zipcode.zip_code'=>$zip), 'recursive' => -1));
		}
		
		if($options['bare']) {
			$data = $data['Zipcode'];
		}
		return $data;
	}
   
   /**
    * 
    * This returns all zip codes (even if one sent)
    * @param $zip
    */
   function get_zip_details($zip) {
   	  if (!$data = $this->find('all',array('conditions'=>array('zip_code'=>$zip)))){ 
         return false;
      } else {
         return $data;       
      }
   }

   function get_details_by_city($cityname) {

   	  if (!$data = $this->find('all', array('conditions'=>array('city LIKE'=>$cityname),'recursive'=>-1))) {
         return false;
      } else {
         return $data;       
      }
   }

   function get_details_by_lat_lon($geolocation) {
   	  if (!$data = $this->find('all', array('conditions'=>array('lat'=>$geolocation['latitude'],'lon'=>$geolocation['longitude']),'recursive'=>-1))) {
         return false;
      } else {
         return $data;       
      }
   }
   function get_details_by_location($locationtext) {
   	
   	  if (!$data = $this->find('all', array('conditions'=>array('locationtext LIKE'=>$locationtext),'recursive'=>-1))) { 
         return false;
      } else {
         return $data;       
      }
   }
   
  function get_details_by_county($county, $state_id){
	   	if (!$data = $this->find('all', array('conditions'=>array('county LIKE'=>$county, 'state_prefix LIKE'=>$state_id),'recursive'=>-1))) { 
	         return false;
	    } else {
	         return $data;       
	    }
    
   }

   
   function get_locations_by_state($stateprefix) {
      if (!$data = $this->find('all', array('conditions'=>array('state_prefix LIKE'=>$stateprefix),'recursive'=>-1,'order'=>array('zip_code'=>'asc')))){
         return false;
      } else {
         return $data;       
      }
   }

   function get_locations_by_county($county) {
  
      if (!$data = $this->find('all', array('conditions'=>array('county LIKE'=>$county),'recursive'=>-1))){

         return false;
      } else {
         return $data;       
      }
   }

   function get_counties_by_state($stateprefix) {
  
      if (!$data = $this->find('all', array('fields'=>array('DISTINCT county'),'conditions'=>array('state_prefix LIKE'=>$stateprefix),'recursive'=>-1,'order'=>array('county'=>'asc')))){
         return false;
      } else {
         return $data;       
      }
   }

   function get_zip_point($zip) {

      // This function pulls just the lattitude and longitude from the
      // database for a given zip code.
      //$sql = "SELECT lat, lon from {$this->tablePrefix}zip_code WHERE zip_code='$zip'";
      //$r = mysql_query($sql);

      if (!$data = $this->find(array('zip_code'=>$zip), array('lat', 'lon'))) {
         //$this->last_error = mysql_error();
         return false;
      } else {
         //$row = mysql_fetch_array($r);
         //mysql_free_result($r);
         //var_dump($data['Zipcode']);
         //return $row;
         return $data['Zipcode'];       
      }      
   }

   
   function calculate_mileage($lat1, $lat2, $lon1, $lon2) {
 
      // used internally, this function actually performs that calculation to
      // determine the mileage between 2 points defined by lattitude and
      // longitude coordinates.  This calculation is based on the code found
      // at http://www.cryptnet.net/fsp/zipdy/
       
      // Convert lattitude/longitude (degrees) to radians for calculations
      $lat1 = deg2rad($lat1);
      $lon1 = deg2rad($lon1);
      $lat2 = deg2rad($lat2);
      $lon2 = deg2rad($lon2);
      
      // Find the deltas
      $delta_lat = $lat2 - $lat1;
      $delta_lon = $lon2 - $lon1;
	
      // Find the Great Circle distance 
      $temp = pow(sin($delta_lat/2.0),2) + cos($lat1) * cos($lat2) * pow(sin($delta_lon/2.0),2);
      $distance = 3956 * 2 * atan2(sqrt($temp),sqrt(1-$temp));

      return $distance;
   }
   
   function get_zips_in_range($zip, $range, $sort=1, $include_base = true, $options = array()) {
   		$default_options = array('datasource' => null, 'conditions' => array());
   		$options = Set::merge($default_options, $options);
   		
      //echo 'kevin';
      // returns an array of the zip codes within $range of $zip. Returns
      // an array with keys as zip codes and values as the distance from 
      // the zipcode defined in $zip.
		//var_dump($zip,$range);
      
	  if(is_array($zip)) {
	  	//passing an array with zip code detail(s) known, don't need to lookup
	  	if(isset($zip['latitude'])) {
	  		$zip['lat'] = $zip['latitude'];
	  		unset($zip['latitude']);
	  	}
	  	if(isset($zip['longitude'])) {
	  		$zip['lon'] = $zip['longitude'];
	  		unset($zip['longitude']);
	  	}
	  	$details = $zip;
	  } else {
     	$details = $this->get_zip_point($zip);  // base zip details
	  }

      if ($details == false) return false;
      
      // This portion of the routine  calculates the minimum and maximum lat and
      // long within a given range.  This portion of the code was written
      // by Jeff Bearer (http://www.jeffbearer.com). This significanly decreases
      // the time it takes to execute a query.  My demo took 3.2 seconds in 
      // v1.0.0 and now executes in 0.4 seconds!  Greate job Jeff!
      
      // Find Max - Min Lat / Long for Radius and zero point and query
      // only zips in that range.
      
     
      $lat_range = $range/69.172;
      $lon_range = abs($range/(cos($details['lat']) * 69.172));
      $min_lat = number_format($details['lat'] - $lat_range, "4", ".", "");
      $max_lat = number_format($details['lat'] + $lat_range, "4", ".", "");
      $min_lon = number_format($details['lon'] - $lon_range, "4", ".", "");
      $max_lon = number_format($details['lon'] + $lon_range, "4", ".", "");

      $return = array();    // declared here for scope

     /* $sql = "SELECT zip_code, lat, lon FROM zip_code ";
      if (!$include_base) $sql .= "WHERE zip_code <> '$zip' AND ";
      else $sql .= "WHERE "; 
      $sql .= "lat BETWEEN '$min_lat' AND '$max_lat' 
               AND lon BETWEEN '$min_lon' AND '$max_lon'"; */
      
      $conditions = array();
      if($range === 'all') {
		// don't want to have a range
		// $conditions = array()
      } else {
	      if(!$include_base) {
	      	$conditions['zip_code'] = "<> $zip";
	      } else {
	      	$conditions['lat BETWEEN ? AND ?'] = array($min_lat, $max_lat);
	      	$conditions['lon BETWEEN ? AND ?'] = array($min_lon, $max_lon);
	      }
      } 
      
      if($options['conditions']) {
      	$conditions = Set::merge($options['conditions'], $conditions);
      }
      
      if($options['datasource']) {
      	$datasource =& ClassRegistry::init($options['datasource'], 'model');
      	$data = $datasource->find('all', array('conditions' =>$conditions, 'recursive' => -1));
      	
      	list($pluginName, $modelName) = pluginSplit($options['datasource']);
      	
      } else {
      	$data = $this->find('all', array('conditions' =>$conditions));
      	$pluginName = null;
      	$modelName = 'Zipcode';
      }
      
      if (!$data) {    // sql error
		//var_dump($data);
         //$this->last_error = mysql_error();
         return false;
         
      } else {
         //return;
         /*while ($row = mysql_fetch_row($r)) {
   
            // loop through all 40 some thousand zip codes and determine whether
            // or not it's within the specified range.
            
            $dist = $this->calculate_mileage($details[0],$row[1],$details[1],$row[2]);
            if ($this->units == _UNIT_KILOMETERS) $dist = $dist * _M2KM_FACTOR;
            if ($dist <= $range) {
               $return[str_pad($row[0], 5, "0", STR_PAD_LEFT)] = round($dist, $this->decimals);
            }
         }
         mysql_free_result($r);*/
      	$zip1VarName = 'zip_code';
      	$zip1 = null;
      	if(isset($details['zip'])) {
      		$zip1 = $details['zip'];
      		$zip1VarName = 'zip';
      	} else if (isset($details['zipcode'])) {
      		$zip1 = $details['zipcode'];
      		$zip1VarName = 'zipcode';
      	} else if (isset($details['zip_code'])) {
      		$zip1 = $details['zip_code'];
      		$zip1VarName = 'zip_code';
      	}
      	
      	$zip2VarName = 'zip_code';
      	
      	$dataByZip = array();
      	foreach($data as $row) {
      		if(isset($row[$modelName]['lat']) and isset($row[$modelName]['lon'])) {
      			$dist = $this->calculate_mileage($details['lat'],$row[$modelName]['lat'],$details['lon'],$row[$modelName]['lon']);
      		} else {
      			$zip2 = null;
      			if(isset($row[$modelName]['zip'])) {
      				$zip2 = $row[$modelName]['zip'];
      				$zip2VarName = 'zip';
      			} else if (isset($row[$modelName]['zipcode'])) {
      				$zip2 = $row[$modelName]['zipcode'];
      				$zip2VarName = 'zipcode';
      			} else if (isset($row[$modelName]['zip_code'])) {
      				$zip2 = $row[$modelName]['zip_code'];
      				$zip2VarName = 'zip_code';
      			}
      			
      			$dist = $this->get_distance($zip1, $zip2);
      		}
      		
            if ($this->units == _UNIT_KILOMETERS) $dist = $dist * _M2KM_FACTOR;
            if ($range === 'all' or $dist <= $range) {
               $return[str_pad($row[$modelName][$zip2VarName], 5, "0", STR_PAD_LEFT)] = round($dist, $this->decimals);
               $dataByZip[str_pad($row[$modelName][$zip2VarName], 5, "0", STR_PAD_LEFT)] = $row;
            }
      	}
      }
      
      // sort array
      switch($sort)
      {
         case _ZIPS_SORT_BY_DISTANCE_ASC:
            asort($return);
            break;
            
         case _ZIPS_SORT_BY_DISTANCE_DESC:
            arsort($return);
            break;
            
         case _ZIPS_SORT_BY_ZIP_ASC:
            ksort($return);
            break;
            
         case _ZIPS_SORT_BY_ZIP_DESC:
            krsort($return);
            break; 
      }
      
      $this->zipInRangeData = $return;
      
      if($options['datasource']) {
      	//merge database results back into return (with distance calculated)
      	$newReturn = array();
      	foreach($return as $zip => $distance) {
      		if(isset($dataByZip[$zip])) {
      			$tmp = $dataByZip[$zip];
      			$tmp['distance'] = $distance;
      			$newReturn[] = $tmp;
      			
	      	}
	      }
	      $return = $newReturn;
      }
      
      if (empty($return)) return false;
      return $return;
   }
   
   function get_detailed_zips_in_range($zip, $range, $sort=1, $include_base = true) {
   	// returns an array of the zip codes within $range of $zip with their details
   	//var_dump($zip,$range);
   
   	$details = $this->get_zip_point($zip);  // base zip details

   	if ($details == false) return false;
   
   	// This portion of the routine  calculates the minimum and maximum lat and
   	// long within a given range.  This portion of the code was written
   	// by Jeff Bearer (http://www.jeffbearer.com). This significanly decreases
   	// the time it takes to execute a query.  My demo took 3.2 seconds in
   	// v1.0.0 and now executes in 0.4 seconds!  Greate job Jeff!
   
   	// Find Max - Min Lat / Long for Radius and zero point and query
   	// only zips in that range.
   	$lat_range = $range/69.172;
   	$lon_range = abs($range/(cos($details['lat']) * 69.172));
   	$min_lat = number_format($details['lat'] - $lat_range, "4", ".", "");
   	$max_lat = number_format($details['lat'] + $lat_range, "4", ".", "");
   	$min_lon = number_format($details['lon'] - $lon_range, "4", ".", "");
   	$max_lon = number_format($details['lon'] + $lon_range, "4", ".", "");
   
   	$return = array();    // declared here for scope
   
   	$conditions = array();
   	$conditions['z_primary'] = 'PRIMARY';
   	if(!$include_base) {
   		$conditions['zip_code'] = "<> $zip";
   	} else {
   		$conditions['lat BETWEEN ? AND ?'] = array($min_lat, $max_lat);
   		$conditions['lon BETWEEN ? AND ?'] = array($min_lon, $max_lon);
   	}
   
   	if (!$data = $this->find('all', array('conditions' =>$conditions))) {    // sql error

   		return false;
   		 
   	} else {
   		foreach($data as $row) {

   			$dist = $this->calculate_mileage($details['lat'],$row['Zipcode']['lat'],$details['lon'],$row['Zipcode']['lon']);
   			//$return['locationdetails'] = $this->find('all',array('conditions'=>array('zip_code'=>$zip)));
   			if ($this->units == _UNIT_KILOMETERS) $dist = $dist * _M2KM_FACTOR;
   			if ($dist <= $range) {
   				$row['Zipcode']['distance'] = round($dist, $this->decimals);
   				$return[] = $row;
   			}
   		}
   	}

   	// sort array
   	function dist_sort($a,$b) {
   		//var_dump($a['Zipcode']['distance']);
   		if($a['Zipcode']['distance'] > $b['Zipcode']['distance'])
   			return 1;//here,if you return -1,return 1 below,the result will be descending
   		if($a['Zipcode']['distance'] < $b['Zipcode']['distance'])
   			return -1;
   		if($a['Zipcode']['distance'] == $b['Zipcode']['distance'])
   			return 0;
   	}
   	switch ($sort) {
   		case '_ZIPS_SORT_BY_DISTANCE_ASC':
   			//asort($return);
   			//var_dump('test');
   			uasort($return, 'dist_sort');
   			
   			break;
   
   		case '_ZIPS_SORT_BY_DISTANCE_DESC':
   			arsort($return);
   			break;
   	}
   
   	$this->zipInRangeData = $return;
   	//var_dump($return);
   	if (empty($return)) return false;
   	return $return;
   }
   

	/**
	* Returns a flat array (e.g. 04072,03801, etc.) for SQL IN statements
    *
    * @return array
    */
	function getZipsInRangeFlat() {
		return array_keys($this->zipInRangeData);
	}
   
   
   function zipRangeOrderCase($zips = array(), $column_name = 'zip') {
   
	   	/*Order by (CASE City
	   	 WHEN 'Paris' 	THEN 1
	   			WHEN 'Chicago' 	THEN 2
	   			WHEN 'Boston' 	THEN 3
	   			WHEN 'New York' THEN 4
	   			WHEN 'Berkeley' THEN 5
	   			WHEN 'Dallas' 	THEN 6
	   			ELSE 100 END) ASC */
	   	$order_by = '';
	   	$zip_count = count($zips);
	   	for($x=0; $x < $zip_count; $x++) {
	   		$order_by .= " WHEN {$zips[$x]} THEN ". ($x + 1);
	   	}
	   
	   	if(!empty($order_by)) {
	   		$order_by = "(CASE $column_name". $order_by ." ELSE 100 END) ASC";
	   		return $order_by;
	   	} else {
	   		return false;
	   	}
   
   }   
	
   	/**
   	 * 
   	 * Takes a string [from a form]
   	 * @param $search string
   	 * @param $options array
   	 * return false if no zip found; or text if a single zip code or an array if multiple zip codes
   	 */
   	function findZipFromText($search, $options = array()) {
		App::import('Sanitize');
		
		$default_options = 
			array(
			'distance' => 0,
			'stateList' => array(), //supply list of states, array('SN' => 'State Name');, to use instead of full geography helper list... will increase performance if you supply a small list
			'defaultState' => Configure::read('avorders.avengerDefaultState'), //array('ME'),	//an array of state abbreviations to use by default when a state isn't found
			'firstMatch' => true, 	//will return first zip code that matches if distance is 0 (otherwise, returns all zipcodes)
			'primary'	=> true,	// set to false to return all zip codes, not just primary zip codes
			);
		$options = am($default_options, $options);
		
		if(is_numeric($search)) {
			//assume it's a zip code
			$zip = trim($search);
			$zip = substr($zip, 0, 5);	//make sure it's only 5 numbers
			
		} else {
			// searching by a string
			// 1. clean/sanitize data
			// 2a. see if preg_match finds a zip code in user-entered text, if so use it
			// 2b. otherwise, explode on spaces, b/c each word needs can be matched, then find unique zip codes
			// 3. create conditions, add in distance condition and zips in range if specified
			// 4. append to join conditions
			
			$location_array = array();
			$location = Sanitize::paranoid($search, array(' ', ',', '.'));
			$location = trim($location);
			
			if(preg_match("/([0-9]{5})(-[0-9]{4})?/i", $location, $match)) {
				//zip code entered with text, use it
				$zip = $match[0];
			} else {
				App::import('Helper', 'Geography');
				$geography = new GeographyHelper();
				
				$terms = array();
				$stateFound = false;
				
				// zip code not in text, so try to find zipcode using city, state if known
				// check to see if there is a comma, if so explode on that: portsmouth, new hampshire
				if(strpos($location, ',') !== false) {
					$location_array = explode(',', $location);
				} else {
					//look for state names (not abbreviations) contained within search term
					$stateAbbr = $geography->isAState($location, array('search' => true, 'list' => $options['stateList']));	// looking to see if a state is buried in this query 
					if($stateAbbr) {
						// state found, set to state_prefix and remove from search terms (for down-the-line processing)
						$stateFound = true;
						$terms['Zipcode.state_prefix'] = $stateAbbr['state'];
						$location = str_ireplace($stateAbbr['term'], '', $location);
						$location = trim($location);
					}
					
					// explode remaining search terms on spaces
					$location_array = explode(' ', $location);
				}
					
				$cityTerms = array();
				foreach($location_array as $term) {
					
					if(!$stateFound) {
						$stateAbbr = $geography->isAState($term);
						if($stateAbbr) {
							//we know this is a state, and the 2 letter abbreviation
							$terms['Zipcode.state_prefix'] = $stateAbbr;
							$stateFound = true;
						} else {
							//not a state, must be a city, eliminated everything else
							$cityTerm = $term .'%';
							if(!empty($cityTerms)) {
								$cityTerm = '%'. $cityTerm;	// if it's not the first word, need to allow double-sided wild card
							}
							$cityTerms[] = $cityTerm;
						}
					} else {
						//state already found, these terms must be a city
						 $cityTerm = $term .'%';
							if(!empty($cityTerms)) {
								$cityTerm = '%'. $cityTerm;	// if it's not the first word, need to allow double-sided wild card
							}
							$cityTerms[] = $cityTerm;
					}
				}
				
				if(!empty($cityTerms)) {
					if(count($cityTerms) == 1) {
						$terms['Zipcode.city LIKE'] = bootArrayFirstValue($cityTerms);
					} else {
						$terms['AND'] = array();
						foreach($cityTerms as $term) {
							$terms['AND'][] = array('Zipcode.city LIKE' => $term);
						}
					}
				}
				
				//default state to ME, b/c this is originally for MyMaineTherapist
				if(!isset($terms['Zipcode.state_prefix'])) {
					$terms['Zipcode.state_prefix'] = $options['defaultState'];
				}
				
				// see if we are checking for primary-only zip codes
				if($options['primary']) {
					$terms['Zipcode.z_primary'] = 'PRIMARY';
				}
				
				$zips = $this->find('all', array('conditions' => $terms, 'fields' => 'Zipcode.city, Zipcode.state_prefix, Zipcode.zip_code, Zipcode.z_primary', 'group' => 'zip_code', 'recursive' => -1));
				if($zips) {
					if($options['firstMatch'] or !empty($options['distance'])) {
						// only want the first zip code (if distance is specified, firstMatch is implied) 
						$zip = bootArrayFirstValue($zips);
						$zip = isset($zip['Zipcode']['zip_code']) ? $zip['Zipcode']['zip_code'] : false;
					} else {
						// for multiple zip codes, you should show a list of available zip codes and allow user to pick zip code in calling function
						$zip = Set::extract('/Zipcode/zip_code', $zips);
					}
				}
			}
		}
		
		//	ADD IN DISTANCE-BASED SEARCHING
		if(!empty($options['distance']) and isset($zip) and $zip) {
			
			if($options['distance'] == 'all') {
				$distance = 'all';
			} else {
				$distance = (int) $options['distance'];
				$distance = ($distance > 100) ? 100 : $distance;	//cap it at 100, that's a lot of zipcodes
			}
				 
			$zipsInRange = $this->get_zips_in_range($zip, $distance);
			if($zipsInRange) {
				$zip = array_keys($zipsInRange);
			}
		} 
		
		
		
		if(!isset($zip) or !$zip) {
			// basically, turning zipcode off, so no results will be found... 
			return false;
		}

		return $zip;
		
	}
	
	
	/**
	 * Performs a Web Service call to FreeGeoIP.net to get IP Whois Information
	 * @param string $ipaddress
	 * @return array $location zip, lat, long, etc. of ip address
	 */
	function fetchFreeGeoIpLocation($ipaddress) {
		//http://freegeoip.net/{format}/{ip_or_hostname}
		// create curl resource
		$location = array();
		
		
		$ipurl = 'http://freegeoip.net/json/'.$ipaddress;
		
		$ch = curl_init();
		// set url
		curl_setopt( $ch, CURLOPT_TIMEOUT_MS, 1500 ); 
		curl_setopt( $ch, CURLOPT_CONNECTTIMEOUT_MS, 1500 ); 
		
		curl_setopt($ch, CURLOPT_URL, $ipurl);
		//return the transfer as a string
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		// $output contains the output string
		$output = curl_exec($ch);
		// close curl resource to free up system resources
		curl_close($ch);
	    if(is_string($output) and !empty($output)){
            $location = json_decode($output, true);
           
            if(!isset($location['zipcode']) or empty($location['zipcode'])){
                //need to find zip code based on geocoded lat/long
                if(isset($location['latitude']) and !empty($location['latitude']) and isset($location['longitude']) and !empty($location['longitude'])) {
                    $zipcode = $this->get_zips_in_range($location, 10);    //no zip code supplied, grab the one that is closest within 10 miles
                    if(is_array($zipcode) and !empty($zipcode)){
                    	$location['zipcode'] = bootArrayFirstValue(array_keys($zipcode));
                    }else{
                    	$location['zipcode'] = '10001';
                    }
                }
            }
            
            if(is_array($location) and !empty($location)) {
	            // change to match format from ZipCode table column names
	            $location['zip_code'] = $location['zipcode'];
	            unset($location['zipcode']);
	            
	            $location['lat'] = $location['latitude'];
	            unset($location['latitude']);
	            
	            $location['lon'] = $location['longitude'];
	            unset($location['longitude']);
	            
	            $location['state_prefix'] = $location['region_code'];
	            unset($location['region_code']);
	            
	            $location['country'] = $location['country_code'];
	            unset($location['country_code']);
	            
	            $location['locationtext'] = $location['city'].', '.$location['state_prefix'];
            }
            
        } else {
            $location = false;
        }
		
		
		return $location;
		
	}
	
	function getClosestLocation($location, $options = array()) {
		$default_options = array('range' => 50);
		$options = Set::merge($default_options, $options);
	
		if($this->hasField('active')) {
			$params['conditions'] = array('AvordersStore.active'=>1);
		}
		return bootArrayFirstValue($this->get_zips_in_range($location, $options['range'], 1, true, null));
	}
    
}