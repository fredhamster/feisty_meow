<?php
/**
 * @var \App\View\AppView $this
 * @var \App\Model\Entity\User $user
 */
use Cake\Log\Log;
?>
<div class="users view large-9 medium-8 columns content">
	<table class="vertical-table">
    
    <?php foreach ($calEvents as $day => $events): ?>
    <tr>
		<th scope="row"><?= __(h($day)) ?></th>
		<td>
			<table>
		    	<?php foreach ( $events as $appt ) : ?>
		           <tr>
					<td><?= h($appt['info'] . ' at ' . $appt['start']); ?></td>
					<!--  <td><?= h('full dump: ' . var_export($appt['event'], true)); ?> -->
				</tr>
			    <?php endforeach; ?>
			</table>
		</td>
	</tr>
    <?php endforeach; ?>
	</table>
</div>
