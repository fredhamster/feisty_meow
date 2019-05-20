<?php
use Migrations\AbstractMigration;
use Cake\Log\Log;
use Cake\ORM\TableRegistry;

/**
 * this is an unusual migration since we want it to turn old data in latlong column into
 * new data in two columns, lat and long.
 */
class TransmogrifyLatLongDataInLocations extends AbstractMigration {
	/**
	 * Change Method.
	 *
	 * More information on this method is available here:
	 * http://docs.phinx.org/en/latest/migrations.html#the-change-method
	 *
	 * @return void
	 */
	// public function change()
	// {
	// }
	
	/**
	 * handle migrating the single column into two.
	 * new columns must already exist.
	 * old latlong column could be blank if we're doing a new db, so we have to handle that.
	 */
	public function up() {
		$locations = TableRegistry::get ( 'Locations' );
		
		$query = $locations->find ();
		
		// iterate across table on latlong.
		foreach ( $query as $locat ) {
			Log::write ( 'debug', 'id=' . var_export ( $locat->id, true ) . ' L&L=' . var_export ( $locat->latlong, true ) );
			// do nothing if the field is empty.
			if ($locat->latlong) {
				// break out the two pieces.
				$latlongExploded = explode ( ',', $locat->latlong );
				Log::write ( 'debug', 'broke into lat=' . $latlongExploded [0] . ' lng=' . $latlongExploded [1] );
				
				// now update the row where we got this info with same lat and long values in two columns.
				$loc_update = $locations->get ( $locat->id );
				$loc_update->lat = floatval ( $latlongExploded [0] );
				$loc_update->lng = floatval ( $latlongExploded [1] );
				$locations->save ( $loc_update );
			}
		}
	}
	
	/**
	 * performs the reverse case of migrating the two columns back into one.
	 * if we don't do this, we can't slide up and down the migration and rollback scale properly while keeping our lat and long data.
	 */
	public function down() {
		$locations = TableRegistry::get ( 'Locations' );
		
		$query = $locations->find ();
		
		// iterate across table on lat and long values.
		foreach ( $query as $locat ) {
			Log::write ( 'debug', 'id=' . var_export ( $locat->id, true ) . ' lat=' . var_export ( $locat->lat, true )
					.' lng=' . var_export ( $locat->lng, true ));
			// do nothing if the fields are empty.
			if ($locat->lat && $locat->lng) {
				// combine the two pieces.
				$latlong = '' . $locat->lat . ',' . $locat->lng; 				
				Log::write ( 'debug', 'combo is=' . $latlong);
				
				// now update the row where we got these values to reproduce the single column.
				$loc_update = $locations->get ( $locat->id );
				$loc_update->latlong = $latlong;
				$locations->save ( $loc_update );
			}
		}
	}
}
