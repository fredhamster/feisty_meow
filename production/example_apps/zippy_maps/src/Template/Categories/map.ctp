<?php
/**
 * @var \App\View\AppView $this
 */

use Cake\Log\Log;
Log::debug('[got into map template]');
?>
<nav class="large-3 medium-4 columns" id="actions-sidebar">
	<ul class="side-nav">
		<li class="heading"><?= __('Actions') ?></li>
		<li><?= $this->Html->link(__('View Category'), ['action' => 'view', $category->id]) ?></li>
		<li><?= $this->Html->link(__('Edit Category'), ['action' => 'edit', $category->id]) ?> </li>
		<li><?= $this->Form->postLink(__('Delete Category'), ['action' => 'delete', $category->id], ['confirm' => __('Are you sure you want to delete # {0}?', $category->id)]) ?> </li>
		<li><?= $this->Html->link(__('List Categories'), ['action' => 'index']) ?> </li>
		<li><?= $this->Html->link(__('New Category'), ['action' => 'add']) ?> </li>
	</ul>
</nav>
<div class="categories view large-9 medium-8 columns content">
	<h3><?= h($category->name) ?></h3>
	<table class="vertical-table">
		<tr>
			<th scope="row"><?= __('Name') ?></th>
			<td><?= h($category->name) ?></td>
		</tr>
		<tr>
			<th scope="row"><?= __('Map of this Category\'s Locations') ?></th>
			<td>
<?php

	// dynamic map preparation below.
	
	// see if clustering is turned on.
	$clustering = in_array ( 'clustering', $selectedList );
	
	// list options we want to override or add for the map.
	$map_options = [ 
		'autoCenter' => true, // automatically encompass markers.
		'clustering' => $clustering,
	];

	// provide the element with all the info it needs to set up the map.
	$element_options = [
		'map_options' => $map_options,
		'map_colors' => 'avmaps/js/sg2_map_colors.js',
		'the_map' => $this->GoogleMap,
		// we want to include the category ID when making ajax calls.
		'ajax_options' => '{ category: "' . $category->id . '"}',
		'default_options' => '{
			iconColor: "' . $this->MapDisplay->defaultIconColor() . '",
			json_provider: "/categories.json",
 		}',
	];
	
	echo $this->element('Avmaps.google_map', $element_options);
	
	// end of dynamic map preparation.
	
?>
			</td>
		</tr>

		<tr>
			<th scope="row"><?= __('View Options') ?></th>
			<td>
            <?= $this->Form->create($category) ?>
			<?php
			echo $this->Form->control ( 'view_options._ids', [ 
					'type' => 'select',
					'multiple' => true,
					'val' => $selectedList,
					'options' => [ 
							'clustering' => 'clustering' 
					] 
			] );
			?>
         
          <?= $this->Form->button(__('Modify View')) ?>
   		 <?= $this->Form->end() ?>
         
            </td>
		</tr>

	</table>
</div>
