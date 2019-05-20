<?php
/**
 * @var \App\View\AppView $this
 */
?>
<nav class="large-3 medium-4 columns" id="actions-sidebar">
	<ul class="side-nav">
		<li class="heading"><?= __('Actions') ?></li>
		<li><?= $this->Html->link(__('List Locations'), ['action' => 'index']) ?></li>
	</ul>
</nav>
<div class="locations form large-9 medium-8 columns content">
    <?= $this->Form->create('Jump') ?>
    <fieldset>
		<legend><?= __('Jump to Location') ?></legend>
        <?php
			if ($id === null) {
				echo h ( 'Sorry, we did not find a location specified as starting point.' );				
			} else {
				// location id is good to go, so let's show the map.
				
				// list the options we want to override or add for the map.
				$map_options = 
				[
					//off now for sure: 'autoCenter' => true, // automatically encompass markers.
					'clustering' => true,  // cluster the markers.
					'zoom' => 12,  // zoom in pretty far but not as a microscope.
					'lat' => $fromLat,
					'lng' => $fromLong,
				];
				
				// provide the element with all the info it needs to set up the map.
				$element_options = 
				[
					'map_options' => $map_options,
					'map_colors' => 'avmaps/js/sg2_map_colors.js',
					'the_map' => $this->GoogleMap,
					// no options to pass to ajax so far...
					'ajax_options' => '{
					}',
					'default_options' => '{
						iconColor: "' . $this->MapDisplay->defaultIconColor() . '",
						json_provider: "/locations.json",
			 		}',
 				];
				
				echo $this->element('Avmaps.google_map', $element_options);				
			}
		?>
    </fieldset>

    <?= $this->Form->end() ?>
</div>
