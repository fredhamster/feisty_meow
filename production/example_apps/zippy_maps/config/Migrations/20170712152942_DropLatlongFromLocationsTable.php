<?php
use Migrations\AbstractMigration;
class DropLatlongFromLocationsTable extends AbstractMigration {
	
	/**
	 * going forward, we have two columns (lat and lng) for the geo coords.  so we're getting rid
	 * of the latlong combined column.
	 */
	public function up() {
		$this->table ( 'locations' )->removeColumn ( 'latlong' )->update ();
	}
	/**
	 * but for preservation of former realities, we re-add the latlong column if we're going backwards.
	 * this supports our code for transmogrifying the coordinates between two column format and one
	 * column fomat.
	 */
	public function down() {
		$this->table ( 'locations' )->addColumn ( 'latlong', 'string', [ 
				'length' => 255,
				'null' => true 
		] )->update ();
	}
}
