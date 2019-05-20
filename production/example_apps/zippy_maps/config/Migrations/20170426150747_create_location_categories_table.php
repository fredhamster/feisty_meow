<?php

use Phinx\Migration\AbstractMigration;

class CreateLocationCategoriesTable extends AbstractMigration
{
    /**
     * Change Method.
     *
     * Write your reversible migrations using this method.
     *
     * More information on writing migrations is available here:
     * http://docs.phinx.org/en/latest/migrations.html#the-abstractmigration-class
     *
     * The following commands can be used in this method and Phinx will
     * automatically reverse them when rolling back:
     *
     *    createTable
     *    renameTable
     *    addColumn
     *    renameColumn
     *    addIndex
     *    addForeignKey
     *
     * Remember to call "create()" or "update()" and NOT "save()" when working
     * with the Table class.
     */
    public function change()
    {
    	$this->table('locations_categories', ['id' => false, 'primary_key' => ['id']])
    	->addColumn('id', 'integer', ['identity' => true, 'signed' => false])
    	->addColumn('location_id', 'integer', ['signed' => false])
    	->addColumn('category_id', 'integer', ['signed' => false])
    	->addColumn('created', 'datetime')
    	->addColumn('modified', 'datetime')
    	->addForeignKey('location_id', 'locations', 'id')
    	//not nullable, array('delete'=> 'SET_NULL', 'update'=> 'NO_ACTION')
    	->addForeignKey('category_id', 'categories', 'id')
    	->create();
    	
    }
}
