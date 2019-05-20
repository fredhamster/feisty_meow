<?php
/**
 * @var \App\View\AppView $this
 */

use Cake\Log\Log;
Log::debug('[got into center template]');
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
	
	//$this->log("state is: " . $state);
	
	// kludgey attempt to focus a little better, based on state size.  
	$small_state = false;
	$medium_state = false;
	// is the state small?
	$small_state = ($state == 'DE' || $state == 'RI' || $state == 'NH');
// 	$this->log('small_state=' . var_export($small_state, true));
	// if not small, we assume medium until told otherwise.
	if (! $small_state) $medium_state = true;
// 	$this->log('medium_state=' . var_export($medium_state, true));	
	// if certain known fat states, then not medium sized.
	if ($state == 'TX' || $state == 'CA' || $state == 'AK') $medium_state = false;	
	$zoomFactor = $small_state? 8 : ( $medium_state? 7 : 5 );
	$this->log('zooming at ' . $zoomFactor);	
	//question: how to pick a more useful zoom factor?  maybe record in db a starting zoom factor per state?
	
	// list options we want to override or add for the map.
	$map_options = [ 
			// automatically encompass markers and auto-center if they haven't told us where to start.
			'autoCenter' => ($mapCenter === null)? true : false,
			'clustering' => $clustering,
			'zoom' => $zoomFactor,
	];

	// start at a specified center if one is given.
	if ($mapCenter) {
		$map_options = array_merge($map_options, $mapCenter);
	}
	
	//$this->log('new options set: ' . var_export($map_options, true));
	
	
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
			json_provider: "/categories.json",
			iconColor: "' . $this->MapDisplay->defaultIconColor() . '",
 		}',
	];
	
	echo $this->element('Avmaps.google_map', $element_options);
	
	/*
	// create the basic map framework using google maps api.
	$this->MapDisplay->setupMap ( $map_options, $this->GoogleMap );
	
	// plug in our chosen color scheme for land, roads, etc.
	$this->MapDisplay->applyColors ( 'avmaps/js/sg2_map_colors.js' );
	
	// after the intial configuration items are done, we can emit the map code.
	// we can still add markers after this point, and do so below.
	$this->MapDisplay->emitMap ( false );
	
	// we were handed a list of locations that match our query, so we can now add them as markers.
	$this->MapDisplay->addMarkersInCategory ( $locationsInCategory, $category );
	
	// set up our event handler for this view.
//	$this->MapDisplay->injectJSFile('avmaps/js/event_trappers.js');
	
	// now we think we are ready to go. let's get the map displayed.
	$this->MapDisplay->finallyDisplayMap ();
	*/
	
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
