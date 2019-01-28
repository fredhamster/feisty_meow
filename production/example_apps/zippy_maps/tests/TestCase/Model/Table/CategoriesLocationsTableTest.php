<?php
namespace App\Test\TestCase\Model\Table;

use App\Model\Table\CategoriesLocationsTable;
use Cake\ORM\TableRegistry;
use Cake\TestSuite\TestCase;

/**
 * App\Model\Table\CategoriesLocationsTable Test Case
 */
class CategoriesLocationsTableTest extends TestCase
{

    /**
     * Test subject
     *
     * @var \App\Model\Table\CategoriesLocationsTable
     */
    public $CategoriesLocations;

    /**
     * Fixtures
     *
     * @var array
     */
    public $fixtures = [
        'app.categories_locations',
        'app.locations',
        'app.categories'
    ];

    /**
     * setUp method
     *
     * @return void
     */
    public function setUp()
    {
        parent::setUp();
        $config = TableRegistry::exists('CategoriesLocations') ? [] : ['className' => 'App\Model\Table\CategoriesLocationsTable'];
        $this->CategoriesLocations = TableRegistry::get('CategoriesLocations', $config);
    }

    /**
     * tearDown method
     *
     * @return void
     */
    public function tearDown()
    {
        unset($this->CategoriesLocations);

        parent::tearDown();
    }

    /**
     * Test initialize method
     *
     * @return void
     */
    public function testInitialize()
    {
        $this->markTestIncomplete('Not implemented yet.');
    }

    /**
     * Test validationDefault method
     *
     * @return void
     */
    public function testValidationDefault()
    {
        $this->markTestIncomplete('Not implemented yet.');
    }

    /**
     * Test buildRules method
     *
     * @return void
     */
    public function testBuildRules()
    {
        $this->markTestIncomplete('Not implemented yet.');
    }
}
