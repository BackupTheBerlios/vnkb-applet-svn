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

function show_newgloss_box($cur_gloss = null)
{
	global $lang_search,$lang_common;
	if ($cur_gloss)
	{
		$source = $cur_gloss['src'];
		$destination = $cur_gloss['dst'];
		$description = $cur_gloss['description'];
	}
	else
	{
		$source = '';
		$destination = '';
		$description = '';
	}
?>
<div id="searchform" class="blockform">
	<h2><span><?php echo '__new gloss' ?></span></h2>
	<div class="box">
		<form id="search" method="post" action="glossary.php">
			<div class="inform">
				<fieldset>
					<legend><?php echo $lang_search['Search criteria legend'] ?></legend>
					<div class="infldset">
						<input type="hidden" name="action" value="newgloss" />
						<label><?php echo '__Source' ?> <input type="text" name="source" size="40" maxlength="100" value="<?php echo pun_htmlspecialchars($source) ?>" /><br /></label>
						<label><?php echo '__Translation' ?> <input type="text" name="destination" size="25" maxlength="25" value="<?php echo pun_htmlspecialchars($destination) ?>" /><br /></label>
						<label><?php echo '__description' ?> <textarea name="description" size="25" maxlength="25"><?php echo pun_htmlspecialchars($description) ?></textarea><br /></label>
					</div>
				</fieldset>
			</div>
			<p><input type="submit" name="search" value="<?php echo $lang_common['Submit'] ?>" accesskey="s" /></p>
		</form>
	</div>
</div>
<?php
}

function process_newgloss(&$cur_gloss)
{
	global $db,$pun_user;
	if (preg_match('#^[\*%]+$#', $cur_gloss['src']) ||
			preg_match('#^[\*%]+$#', $cur_gloss['dst']))
	{
		$cur_gloss['error'] = '* and % are reserved for searching. Please do not use them.';
		return 0;
	}
	$now = time();
	$result = $db->query('INSERT INTO '.$db->prefix.'glossary_items (user_id,username,ctime,mtime,src,dst,description) VALUES ('.$pun_user['id'].',\''.$pun_user['username'].'\',\''.$now.'\',\''.$now.'\',\''.$db->escape($cur_gloss['src']).'\',\''.$db->escape($cur_gloss['dst']).'\',\''.$db->escape($cur_gloss['description']).'\')') or error('Unable to insert new glossary item', __FILE__, __LINE__, $db->error());
	return $db->insert_id();
}

function show_gloss_item($cur_gloss)
{
	global $pun_config,$pun_user;
	include PUN_ROOT.'lang/'.$pun_user['language'].'/topic.php';
?>
<div class="blockpost">
	<h2><?php echo '__Glossary:'.$cur_gloss['src'];?></h2>
	<div class="box">
		<div class="inbox">
			<div class="postleft">
				<dl>
					<dd>Author: <strong><?php echo $cur_gloss['username'] ?></strong></dd>
					<dd>Created: <strong><?php echo date($pun_config['o_date_format'],$cur_gloss['ctime']) ?></strong></dd>
					<dd>Last modified: <strong><?php echo date($pun_config['o_date_format'],$cur_gloss['mtime']) ?></strong></dd>
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
			<div class="postfootright">
				<ul>
					<li><a href="glossitem.php?action=history&id=<?php echo $cur_gloss['id'] ?>">History</a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
<?php if ($cur_gloss['topic_id'] == 0): ?>
					<li><a href="post.php?evgs=<?php echo $cur_gloss['id'] ?>&fid=<?php echo $pun_config['o_evgs_forum'] ?>">Comment</a></li>
<?php else: ?>
					<li><a href="post.php?tid=<?php echo $cur_gloss['topic_id'] ?>">Comment</a></li>
<?php endif; ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=edit&id=<?php echo $cur_gloss['id'] ?>">Edit</a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=delete&id=<?php echo $cur_gloss['id'] ?>">Delete</a></li>
				</ul>
			</div>
		</div>
	</div>
</div>
<?php
}
