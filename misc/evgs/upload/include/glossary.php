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
	show_gloss_item($cur_gloss);
}

function show_newgloss_box($cur_gloss = null,$edit_mode = false)
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
<?php if ($edit_mode): ?>
		<form id="editgloss" method="post" action="glossitem.php?id=<?php echo $cur_gloss['id'] ?>">
		<input type="hidden" name="action" value="editgloss" />
<?php else: ?>
		<form id="search" method="post" action="glossary.php">
		<input type="hidden" name="action" value="newgloss" />
<?php endif; ?>
			<div class="inform">
				<fieldset>
					<legend><?php echo $lang_search['Search criteria legend'] ?></legend>
					<div class="infldset">
						<label><?php echo '__Source' ?> <input type="text" name="source" size="40" maxlength="100" value="<?php echo pun_htmlspecialchars($source) ?>" <?php if ($edit_mode) echo 'disabled="disabled"'; ?> /><br /></label>
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

function process_newgloss(&$cur_gloss,$edit_mode = false)
{
	global $db,$pun_user,$lang_common;
	if (preg_match('#^[\*%]+$#', $cur_gloss['src']) ||
			preg_match('#^[\*%]+$#', $cur_gloss['dst']))
	{
		$cur_gloss['error'] = '* and % are reserved for searching. Please do not use them.';
		return 0;
	}
	$now = time();
	if ($edit_mode)
	{
		$result = $db->query('SELECT * FROM '.$db->prefix.'glossary_items WHERE id='.$cur_gloss['id']) or error('Unable to fetch glossary item', __FILE__, __LINE__, $db->error());
		if (!$db->num_rows($result))
			message($lang_common['Bad request']);
		$old_gloss = $db->fetch_assoc($result);
		$result = $db->query('INSERT INTO '.$db->prefix.'glossrev_items (id,rev_id,topic_id,src,dst,user_id,username,ctime,mtime,description) VALUES ('.$old_gloss['id'].','.$old_gloss['rev_id'].','.$old_gloss['topic_id'].',\''.$db->escape($old_gloss['src']).'\',\''.$db->escape($old_gloss['dst']).'\','.$old_gloss['user_id'].',\''.$db->escape($old_gloss['username']).'\','.$old_gloss['ctime'].','.$old_gloss['mtime'].',\''.$db->escape($old_gloss['description']).'\')') or error('Unable to back up glossary item', __FILE__, __LINE__, $db->error());
		$result = $db->query('UPDATE '.$db->prefix.'glossary_items SET rev_id=rev_id+1, user_id='.$pun_user['id'].', username=\''.$pun_user['username'].'\', mtime='.$now.', dst=\''.$db->escape($cur_gloss['dst']).'\', description=\''.$db->escape($cur_gloss['description']).'\'') or error('Unable to update glossary item', __FILE__, __LINE__, $db->error());
	}
	else
		$result = $db->query('INSERT INTO '.$db->prefix.'glossary_items (user_id,username,ctime,mtime,src,dst,description) VALUES ('.$pun_user['id'].',\''.$pun_user['username'].'\',\''.$now.'\',\''.$now.'\',\''.$db->escape($cur_gloss['src']).'\',\''.$db->escape($cur_gloss['dst']).'\',\''.$db->escape($cur_gloss['description']).'\')') or error('Unable to insert new glossary item', __FILE__, __LINE__, $db->error());
	return $db->insert_id();
}

function show_gloss_item($cur_gloss,$history_mode = false)
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
					<dd>Revision: <strong><?php echo $cur_gloss['rev_id'] ?></strong></dd>
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
<?php if ($history_mode): ?>
					<li><a href="glossitem.php?action=revdelete&id=<?php echo $cur_gloss['id'] ?>&rev_id=<?php echo $cur_gloss['rev_id'] ?>">Delete</a></li>
<?php else: ?>
					<li><a href="glossitem.php?action=history&id=<?php echo $cur_gloss['id'] ?>">History</a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
<?php if ($cur_gloss['topic_id'] == 0): ?>
					<li><a href="post.php?evgs=<?php echo $cur_gloss['id'] ?>&fid=<?php echo $pun_config['o_evgs_forum'] ?>">Comment</a></li>
<?php else: ?>
					<li><a href="viewtopic.php?id=<?php echo $cur_gloss['topic_id'] ?>">View comments</a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="post.php?tid=<?php echo $cur_gloss['topic_id'] ?>">Comment</a></li>
<?php endif; ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=edit&id=<?php echo $cur_gloss['id'] ?>">Edit</a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=delete&id=<?php echo $cur_gloss['id'] ?>">Delete</a></li>
<?php endif; ?>
				</ul>
			</div>
		</div>
	</div>
</div>
<?php
}
