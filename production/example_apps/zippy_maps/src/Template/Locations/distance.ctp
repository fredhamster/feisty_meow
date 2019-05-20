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
    <?= $this->Form->create('DistanceCalculator') ?>
    <fieldset>
        <legend><?= __('Calculate Distance') ?></legend>
        <?php
			echo $this->Form->control('from._ids', [
	    	'type' => 'select',
    		'options' => $locationsFrom,
    		'value' => $fromId,
			]);
			
			if ($fromAddress !== null) {
				echo $this->Form->control('from-address', [
					'type' => 'textarea',
					'rows' => 1,
					'default' => $fromAddress,
					'disabled' => 'disabled'
				]);				
			}
            
			echo $this->Form->control('to._ids', [
	    	'type' => 'select',
    		'options' => $locationsTo,
    		'value' => $toId,
			]);

			if ($toAddress !== null) {
				echo $this->Form->control('to-address', [
					'type' => 'textarea',
					'rows' => 1,
					'default' => $toAddress,
					'disabled' => 'disabled'
				]);				
			}
			
			echo $this->Form->control('distance', [
				'type' => 'textarea',
				'rows' => 1,
				'default' => $distanceCalculated,
				'disabled' => 'disabled'
			]);
			            
        ?>
    </fieldset>
    <?= $this->Form->button(__('Calculate')) ?>
    <?= $this->Form->end() ?>
</div>
