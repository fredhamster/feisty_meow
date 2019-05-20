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
    <?= $this->Form->create('RadiusFinder') ?>
    <fieldset>
		<legend><?= __('Find Locations Within Radius') ?></legend>
        <?php
			if ($id === null) {
				echo h ( 'Sorry, we did not find a location specified as starting point.' );				
			} else {
				echo $this->Form->control ( 'from-address', [ 
						'type' => 'textarea',
						'rows' => 1,
						'default' => $fromAddress,
						'disabled' => 'disabled' 
				] );
				
				echo $this->Form->control ( 'radius (in miles)' , ['default' => $radius]);
								
				// list options we want to override or add for the map.
				$map_options = [
					'autoCenter' => true, // automatically encompass markers.
					'clustering' => true,  // cluster the markers.
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
					// not passing json provider, since we are doing a special radius thing here.  so no updates.
					'default_options' => '{
						iconColor: "' . $this->MapDisplay->defaultIconColor() . '",
			 		}',
			 	];
								
				echo $this->element('Avmaps.google_map', $element_options);
			}
		?>
    </fieldset>
    <?= $this->Form->button(__('Calculate')) ?>
    <?= $this->Form->end() ?>
</div>
