<?php
use Migrations\AbstractMigration;

class AddLatLongColumnsToLocationsTable extends AbstractMigration
{
    /**
     * Change Method.
     *
     * More information on this method is available here:
     * http://docs.phinx.org/en/latest/migrations.html#the-change-method
     * @return void
     */
    public function change()
    {
        $table = $this->table('locations');
        
       	$table->addColumn('lat', 'float', ['null' => true]);
       	$table->addColumn('lng', 'float', ['null' => true]);
        
        $table->update();
        
        
    }
}
