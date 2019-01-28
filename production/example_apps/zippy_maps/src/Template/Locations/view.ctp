<?php
use Avmaps\Controller\Component\SimpleMapsComponent;

/**
  * @var \App\View\AppView $this
  */
?>
<nav class="large-3 medium-4 columns" id="actions-sidebar">
    <ul class="side-nav">
        <li class="heading"><?= __('Actions') ?></li>
        <li><?= $this->Html->link(__('Edit Location'), ['action' => 'edit', $location->id]) ?> </li>
        <li><?= $this->Form->postLink(__('Delete Location'), ['action' => 'delete', $location->id], ['confirm' => __('Are you sure you want to delete # {0}?', $location->id)]) ?> </li>
        <li><?= $this->Html->link(__('List Locations'), ['action' => 'index']) ?> </li>
        <li><?= $this->Html->link(__('New Location'), ['action' => 'add']) ?> </li>
    </ul>
</nav>
<div class="locations view large-9 medium-8 columns content">
    <h3><?= h($location->name) ?></h3>
    <table class="vertical-table">
        <tr>
            <th scope="row"><?= __('Name') ?></th>
            <td><?= h($location->name) ?></td>
        </tr>
        <tr>
            <th scope="row"><?= __('Location') ?></th>
            <td><?= h($location->location) ?></td>
            <td>
            			<?php

        //beginning of static map junk...
        $addresses = [
        		[ 'address' => $location->lat
        		. ',' . $location->lng 
        		],
        ];
                
        $marker_options = [
        		'color' => $this->MapDisplay->defaultIconColor(),
        		'shadow' => 'false'
        ];
        
        $markers = $this->GoogleMap->staticMarkers($addresses, $marker_options);
        
        $map_options = [
        		'center' => $location->lat
        		. ',' . $location->lng,
        		'markers' => $markers,
        		'zoom' => 8,
        ];
        
        $map = $this->GoogleMap->staticMap($map_options);
        
        echo $map;
        
        $this->GoogleMap->finalize();
        
        //end static map junk.
			?>
            
            </td>
        </tr>
        <tr>
            <th scope="row"><?= __('Lat&Long') ?></th>
            <td><?= h($location->lat . ',' . $location->lng) ?></td>
        </tr>
        <tr>
            <th scope="row"><?= __('Zip Code (RGC)') ?></th>
            <td>
            <?php
            $results = SimpleMapsComponent::reverseGeocode($location->lat, $location->lng, [
            	'key' => $api_key
            ]);
            if ($results) {
				echo h($results[0]);
            } else {
            	echo h('failed to reverse geocode');
            }

            ?>
        </tr>
        <tr>
            <th scope="row"><?= __('Address (RGC)') ?></th>
            <td><?= h($results[1]) ?></td>
        </tr>
        <tr>
        <th scope="row"><?= __('Categories') ?></th>
        <td>
        <?= $this->Form->control('categories._ids', [
	    	'type' => 'select',
		    'multiple' => true,
		    'val' => $selectedList,
    		'options' => $categoriesList,
			]);
			?>
		</td>
    </table>
</div>
