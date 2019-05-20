<?php
namespace App\Model\Entity;

use Cake\ORM\Entity;

/**
 * CategoriesLocation Entity
 *
 * @property int $id
 * @property int $location_id
 * @property int $category_id
 * @property \Cake\I18n\Time $created
 * @property \Cake\I18n\Time $modified
 *
 * @property \App\Model\Entity\Location $location
 * @property \App\Model\Entity\Category $category
 */
class CategoriesLocation extends Entity
{

    /**
     * Fields that can be mass assigned using newEntity() or patchEntity().
     *
     * Note that when '*' is set to true, this allows all unspecified fields to
     * be mass assigned. For security purposes, it is advised to set '*' to false
     * (or remove it), and explicitly make individual fields accessible as needed.
     *
     * @var array
     */
    protected $_accessible = [
        '*' => true,
        'id' => false
    ];
}
