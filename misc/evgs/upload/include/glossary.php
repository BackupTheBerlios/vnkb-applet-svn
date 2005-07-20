<?php
function glossary_item_div($topic_id)
{
	global $db,$pun_user,$pun_config,$lang_topic,$lang_common;

	$result = $db->query('SELECT * FROM '.$db->prefix.'glossary_items WHERE topic_id='.$topic_id) or error('Could not fetch glossary item info',__FILE__,__LINE__,$db->error());
	if (!$db->num_rows($result))
		return;
	$cur_gloss = $db->fetch_assoc($result);
	$cur_gloss['description'] = parse_message($cur_gloss['description'],'1');
	$username = '<a href="profile.php?id='.$cur_gloss['user_id'].'">'.pun_htmlspecialchars($cur_gloss['username']).'</a>';

?>
<div class="blockpost">
	<h2><?php echo '__Glossary';?></h2>
	<div class="box">
		<div class="inbox">
			<div class="postleft">
				<dl>
					<dt><strong><?php echo $username ?></strong></dt>
					<dd class="usertitle"><strong><?php echo $user_title ?></strong></dd>
					<dd class="postavatar"><?php echo $user_avatar ?></dd>
<?php if (count($user_info)) echo "\t\t\t\t\t".implode('</dd>'."\n\t\t\t\t\t", $user_info).'</dd>'."\n"; ?>
<?php if (count($user_contacts)) echo "\t\t\t\t\t".'<dd class="usercontacts">'.implode('&nbsp;&nbsp;', $user_contacts).'</dd>'."\n"; ?>
				</dl>
			</div>
			<div class="postright">
				<h3><?php echo $cur_gloss['src'].' '.$cur_gloss['dst']; ?></h3>
				<div class="postmsg">
					<dl>
						<dd>__Source: <?php echo pun_htmlspecialchars($cur_gloss['src']); ?></dd>
						<dd>__Translation: <?php echo pun_htmlspecialchars($cur_gloss['dst']); ?></dd>
					</dl>
					<?php echo $cur_gloss['description']; ?>
				</div>
			</div>	
			<div class="clearer"></div>
			<div class="postfootleft"><?php if ($cur_gloss['user_id'] > 1) echo '<p>'.$is_online.'</p>'; ?></div>
			<div class="postfootright"><div>&nbsp;</div></div>
		</div>
	</div>
</div>
<?php
}
