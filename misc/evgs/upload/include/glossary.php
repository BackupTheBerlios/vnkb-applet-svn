<?php
function evgs_item_div($topic_id)
{
	global $db,$pun_user,$pun_config,$lang_topic,$lang_common;

	$result = $db->query('SELECT * FROM '.$db->prefix.'glossary_items WHERE topic_id='.$topic_id) or error('Could not fetch glossary item info',__FILE__,__LINE__,$db->error());
	if (!$db->num_rows($result))
		return;
	$cur_gloss = $db->fetch_assoc($result);
	$cur_gloss['description'] = parse_message($cur_gloss['description'],'1');
	$username = '<a href="profile.php?id='.$cur_gloss['user_id'].'">'.pun_htmlspecialchars($cur_gloss['username']).'</a>';
	show_gloss_item($cur_gloss,'viewtopic');
}

function show_newgloss_box($cur_gloss = null,$edit_mode = false)
{
	global $lang_search,$lang_common,$pun_user;
	include PUN_ROOT.'lang/'.$pun_user['language'].'/evgs.php';
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
	<h2><span><?php echo $lang_evgs['New glossary'] ?></span></h2>
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
					<legend><?php echo $lang_evgs['Glossary information'] ?></legend>
					<div class="infldset">
						<label><?php echo $lang_evgs['Source'] ?> <input type="text" name="source" size="40" maxlength="100" value="<?php echo pun_htmlspecialchars($source) ?>" <?php if ($edit_mode) echo 'disabled="disabled"'; ?> /><br /></label>
						<label><?php echo $lang_evgs['Translation'] ?> <input type="text" name="destination" size="25" maxlength="25" value="<?php echo pun_htmlspecialchars($destination) ?>" /><br /></label>
						<label><?php echo $lang_evgs['Description'] ?> <textarea name="description" size="25" maxlength="25"><?php echo pun_htmlspecialchars($description) ?></textarea><br /></label>
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
		$result = $db->query('UPDATE '.$db->prefix.'glossary_items SET rev_id=rev_id+1, user_id='.$pun_user['id'].', username=\''.$pun_user['username'].'\', mtime='.$now.', dst=\''.$db->escape($cur_gloss['dst']).'\', description=\''.$db->escape($cur_gloss['description']).'\' WHERE id='.$cur_gloss['id']) or error('Unable to update glossary item', __FILE__, __LINE__, $db->error());
	}
	else
		$result = $db->query('INSERT INTO '.$db->prefix.'glossary_items (user_id,username,ctime,mtime,src,dst,description) VALUES ('.$pun_user['id'].',\''.$pun_user['username'].'\',\''.$now.'\',\''.$now.'\',\''.$db->escape($cur_gloss['src']).'\',\''.$db->escape($cur_gloss['dst']).'\',\''.$db->escape($cur_gloss['description']).'\')') or error('Unable to insert new glossary item', __FILE__, __LINE__, $db->error());
	return $db->insert_id();
}

function show_gloss_item($cur_gloss,$mode = 'glossary')
{
	global $pun_config,$pun_user,$lang_common;
	include PUN_ROOT.'lang/'.$pun_user['language'].'/topic.php';
	include PUN_ROOT.'lang/'.$pun_user['language'].'/evgs.php';
	$dsts = parse_evgs_destination($cur_gloss['dst']);
	$votes = parse_evgs_votes($cur_gloss['votes'],$dsts);
?>
<div class="blockpost">
	<h2><?php printf($lang_evgs['Glossary: %s'],$cur_gloss['src']) ?></h2>
	<div class="box">
		<div class="inbox">
			<div class="postleft">
				<dl>
					<dd><?php echo $lang_common['Author'] ?>: <strong><?php echo $cur_gloss['username'] ?></strong></dd>
					<dd><?php echo $lang_evgs['Created'] ?>: <strong><?php echo date($pun_config['o_date_format'],$cur_gloss['ctime']) ?></strong></dd>
					<dd><?php echo $lang_evgs['Revision'] ?>: <strong><?php echo $cur_gloss['rev_id'] ?></strong></dd>
					<dd><?php echo $lang_evgs['Last modified'] ?>: <strong><?php echo date($pun_config['o_date_format'],$cur_gloss['mtime']) ?></strong></dd>
				</dl>
			</div>
			<div class="postright">
				<h3><?php echo $cur_gloss['src'].' '.$cur_gloss['dst']; ?></h3>
				<div class="postmsg">
					<dl>
						<dd><?php echo $lang_evgs['Source'] ?>: <?php echo pun_htmlspecialchars($cur_gloss['src']); ?></dd>
<?php if (count($dsts) == 1): ?>
						<dd><?php echo $lang_evgs['Translation']?>: <?php echo pun_htmlspecialchars($dsts[0]); ?></dd>
<?php else: ?>
						<dd><?php echo $lang_evgs['Translation']?>:
							<ol class="orderlist">
<?php	foreach ($dsts as $idx => $dst): ?>
								<li><?php echo pun_htmlspecialchars($idx.'. '.$dst); ?>&nbsp;<?php if ($votes['count'] > 0) echo ' ('.($votes[$idx]).')'?></li>
<?php 	endforeach; ?>
							</ol>
						</dd>
<?php endif; ?>
					</dl>
					<?php echo $cur_gloss['description']; ?>
				</div>
			</div>	
			<div class="clearer"></div>
			<div class="postfootright">
<?php	if (!$pun_user['is_guest']): ?>
				<ul>
<?php		if ($mode == 'history'): ?>
					<li><a href="glossitem.php?action=revdelete&id=<?php echo $cur_gloss['id'] ?>&rev_id=<?php echo $cur_gloss['rev_id'] ?>"><?php echo $lang_evgs['Delete']?></a></li>
<?php		else: ?>
					<li><a href="glossitem.php?action=history&id=<?php echo $cur_gloss['id'] ?>"><?php echo $lang_evgs['History']?></a></li>
<?php			if ($cur_gloss['topic_id'] == 0): ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="post.php?evgs=<?php echo $cur_gloss['id'] ?>&fid=<?php echo $pun_config['o_evgs_forum'] ?>"><?php echo $lang_evgs['Comment']?></a></li>
<?php			else: ?>
<?php				if ($mode == 'glossary'): ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="viewtopic.php?id=<?php echo $cur_gloss['topic_id'] ?>"><?php echo $lang_evgs['View comments']?></a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="post.php?tid=<?php echo $cur_gloss['topic_id'] ?>"><?php echo $lang_evgs['Comment']?></a></li>
<?php				endif; ?>
<?php			endif; ?>
<?php if (count($dsts) > 1): ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=vote&id=<?php echo $cur_gloss['id'] ?>"><?php echo $lang_evgs['Vote']?></a></li>
<?php endif; ?>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=edit&id=<?php echo $cur_gloss['id'] ?>"><?php echo $lang_evgs['Edit']?></a></li>
					<li><?php echo $lang_topic['Link separator'] ?></li>
					<li><a href="glossitem.php?action=delete&id=<?php echo $cur_gloss['id'] ?>"><?php echo $lang_evgs['Delete']?></a></li>
<?php		endif; ?>
<?php	endif; ?>
				</ul>
			</div>
		</div>
	</div>
</div>
<?php
}

function parse_evgs_destination($destination)
{
	return explode('|',$destination);
}
function parse_evgs_votes($votes,$dsts)
{
	$rdsts = array();
	$result = array();
	foreach ($dsts as $idx => $dst)
	{
		$rdsts[$dst] = $idx;
		$result[$idx] = 0;
	}
	$result['count'] = 0;
	
	if ($votes != '')
	{
		$votedata = unserialize($votes);
		foreach ($votedata as $uid => $data)
		{
			if (isset($rdsts[$data]))
			{
				$result[$rdsts[$data]] ++;
				$result['count'] ++;
			}
		}
	}
	return $result;
}

function evgs_quick_links()
{
	global $pun_config,$tpl_main,$pun_user;
	// START SUBST - <pun_quick_links>
	if ($pun_config['o_quick_links'] == '1')
	{
		include PUN_ROOT.'lang/'.$pun_user['language'].'/evgs.php';
		ob_start();
		$quick_links = $pun_config['o_quick_links_content'];
		$quick_links = preg_replace_callback('/\{([^|}]*)(\|([^}]*))?\}/','quick_links_callback',$quick_links);
	
?>
<div id="quick_links" class="block">
	<h2><span><?php echo $lang_evgs['Quick links'] ?></span></h2>
	<div class="box">
		<div class="inbox">
			<div><?php echo $quick_links ?></div>
		</div>
	</div>
</div>
<?php

	$tpl_temp = trim(ob_get_contents());
	$tpl_main = str_replace('<pun_quick_links>', $tpl_temp, $tpl_main);
	ob_end_clean();
}
else
	$tpl_main = str_replace('<pun_quick_links>', '', $tpl_main);
// END SUBST - <pun_quick_links>
}

function quick_links_callback($matches)
{
	if ($matches[1] == '') {
		return '
<script type="text/javascript" src="style/drupal.js"></script>
<script type="text/javascript" src="style/autocomplete.js"></script>
<form action="glossary.php" type="post">
<input class="autocomplete" type="hidden" id="edit-ing-autocomplete" value="glossajax.php" disabled="disabled" />
<input type="text" maxlength="20" class="form-text form-autocomplete" name="source" id="edit-ing" size="10" value="" />
<input type="hidden" name="action" value="search" />
</form>';
	} else
		return '<a href="glossary.php?action=search&source='.pun_htmlspecialchars($matches[1]).'">'.(isset($matches[3]) ? $matches[3] : pun_htmlspecialchars($matches[1])).'</a>';
}

function evgs_footer()
{
	global $footer_style,$pun_user,$lang_common;
	if ($footer_style == 'glossary')
	{
		include PUN_ROOT.'lang/'.$pun_user['language'].'/evgs.php';
		if (!$pun_user['is_guest'])
		{
			echo "\n\t\t\t".'<dl id="searchlinks" class="conl">'."\n\t\t\t\t".'<dt><strong>'.$lang_common['Search links'].'</strong></dt>'."\n\t\t\t\t".'<dd><a href="glossary.php?action=show_24h">'.$lang_evgs['Show recent items'].'</a></dd>'."\n";
			echo "\t\t\t\t".'<dd><a href="glossary.php?action=show_unanswered">'.$lang_evgs['Show uncommented items'].'</a></dd>'."\n\t\t\t</dl>";
		}
		else
		{
			echo "\n\t\t\t".'<dl id="searchlinks" class="conl">'."\n\t\t\t\t".'<dt><strong>'.$lang_common['Search links'].'</strong></dt><dd><a href="glossary.php?action=show_24h">'.$lang_evgs['Show recent items'].'</a></dd>'."\n";
			echo "\t\t\t\t".'<dd><a href="glossary.php?action=show_unanswered">'.$lang_evgs['Show uncommented items'].'</a></dd>'."\n\t\t\t".'</dl>'."\n";
		}
	}
}

function evgs_config_check()
{
	global $pun_config,$db;
	$err = array();
	if (!isset($pun_config['o_quick_links']))
		$err[] = 'o_quick_links is not available';
	if (!isset($pun_config['o_quick_links_content']))
		$err[] = 'o_quick_links_content is not available';
	if (!isset($pun_config['o_evgs_forum']))
		$err[] = 'o_evgs_forum is not available';
	else
	{
		$result = $db->query('SELECT * FROM '.$db->prefix.'forums WHERE id='.$pun_config['o_evgs_forum']);
		if ($db->num_rows($result) == 0)
			$err[] = 'Invalid Comment forum';
	}
	return $err;
}

function evgs_config_repair()
{
	global $pun_config,$db;
	$sql = array();
	if (!isset($pun_config['o_quick_links']))
		$sql[] = 'INSERT INTO '.$db->prefix.'config SET conf_name=\'o_quick_links\', conf_value=0;';
	if (!isset($pun_config['o_quick_links_content']))
		$sql[] = 'INSERT INTO '.$db->prefix.'config SET conf_name=\'o_quick_links_content\', conf_value=\''.$db->escape('<table class="tablelayout">
<tr>
<td align="center">{A*|A}</td>
<td align="center">{B*|B}</td>
<td align="center">{C*|C}</td>
<td align="center">{D*|D}</td>
<td align="center">{E*|E}</td>
<td align="center">{F*|F}</td>
<td align="center">{G*|G}</td>
<td align="center">{H*|H}</td>
<td align="center">{I*|I}</td>
<td align="center">{J*|J}</td>
<td align="center">{K*|K}</td>
<td align="center">{L*|L}</td>
<td align="center">{M*|M}</td>
<td align="center">{N*|N}</td>
<td align="center">{O*|O}</td>
<td align="center">{P*|P}</td>
<td align="center">{Q*|Q}</td>
<td align="center">{R*|R}</td>
<td align="center">{S*|S}</td>
<td align="center">{T*|T}</td>
<td align="center">{U*|U}</td>
<td align="center">{V*|V}</td>
<td align="center">{W*|W}</td>
<td align="center">{X*|X}</td>
<td align="center">{Y*|Y}</td>
<td align="center">{Z*|Z}</td>
<td width="100px">{}</td>
</tr>
</table>').'\';';
	if (!isset($pun_config['o_evgs_forum']))
		$sql[] = 'INSERT INTO '.$db->prefix.'config SET conf_name=\'o_evgs_forum\', conf_value=0;';
	return $sql;
}

function evgs_table_check()
{
	return array();
}

function evgs_table_repair()
{
	global $db;
	$sql = array();
	$sql[] = "CREATE TABLE `".$db->prefix."glossary_items` (
		`id` int(10) unsigned NOT NULL auto_increment,
	`rev_id` int(10) unsigned NOT NULL default '1',
	`topic_id` int(10) unsigned NOT NULL default '0',
	`src` varchar(100) NOT NULL default '',
	`dst` varchar(100) NOT NULL default '',
	`user_id` mediumint(10) unsigned NOT NULL default '0',
	`username` varchar(200) NOT NULL default '',
	`ctime` int(10) unsigned NOT NULL default '0',
	`mtime` int(10) unsigned NOT NULL default '0',
	`description` text NOT NULL,
	`votes` text NOT NULL,
	PRIMARY KEY  (`id`)
	) TYPE=MyISAM;";

	$sql[] = "CREATE TABLE `".$db->prefix."glossrev_items` (
		`id` int(10) unsigned NOT NULL default '0',
	`rev_id` int(10) unsigned NOT NULL default '0',
	`topic_id` int(10) unsigned NOT NULL default '0',
	`src` varchar(100) NOT NULL default '',
	`dst` varchar(100) NOT NULL default '',
	`user_id` mediumint(10) unsigned NOT NULL default '0',
	`username` varchar(200) NOT NULL default '',
	`ctime` int(10) unsigned NOT NULL default '0',
	`mtime` int(10) unsigned NOT NULL default '0',
	`description` text NOT NULL,
	PRIMARY KEY  (`id`,`rev_id`)
	) TYPE=MyISAM;";
	return $sql;
}

function evgs_file_check()
{
	return array();

}

function evgs_file_repair()
{
	return array();
}
