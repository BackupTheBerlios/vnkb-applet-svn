<?php
/***********************************************************************

  Copyright (C) 2002-2005  Rickard Andersson (rickard@punbb.org)

  This file is part of PunBB.

  PunBB is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  PunBB is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
  MA  02111-1307  USA

************************************************************************/


// The contents of this file are very much inspired by the file search.php
// from the phpBB Group forum software phpBB2 (http://www.phpbb.com).


define('PUN_ROOT', './');
require PUN_ROOT.'include/common.php';


// Load the search.php language file
require PUN_ROOT.'lang/'.$pun_user['language'].'/search.php';
require PUN_ROOT.'include/glossary.php';


if ($pun_user['g_read_board'] == '0')
	message($lang_common['No view']);
else if ($pun_user['g_search'] == '0')
	message($lang_search['No search permission']);


// Detect two byte character sets
$multibyte = (isset($lang_common['lang_multibyte']) && $lang_common['lang_multibyte']) ? true : false;

if (isset($_POST['action']) && $_POST['action'] == 'newgloss')
{
	if ($pun_user['is_guest'])
		message($lang_common['No permission']);
	$cur_gloss = array(
		'src' => (isset($_POST['source'])) ? strtolower(trim($_POST['source'])) : null,
		'dst' => (isset($_POST['destination'])) ? strtolower(trim($_POST['destination'])) : null,
		'description' => (isset($_POST['description'])) ? strtolower(trim($_POST['description'])) : null
		);
	$id = process_newgloss($cur_gloss);
	if ($id > 0)
		redirect('glossary.php?action=search&source='.urlencode($cur_gloss['src']),'__Added successfully new term');
	else
	{
		if (!defined('PUN_HEADER'))
		{
			global $pun_user;
	
			$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / '.$lang_common['Info'];
			require PUN_ROOT.'header.php';
		}
		if (isset($cur_gloss['error']))
		{
?>
<div id="msg" class="block">
	<h2><span><?php echo $lang_common['Info'] ?></span></h2>
	<div class="box">
		<div class="inbox">
		<p><?php echo $cur_gloss['error'] ?></p>
<?php if (!$no_back_link): ?>		<p><a href="javascript: history.go(-1)"><?php echo $lang_common['Go back'] ?></a></p>
<?php endif; ?>		</div>
	</div>
</div>
<?php
		}
		show_newgloss_box($cur_gloss);
		require PUN_ROOT.'footer.php';
	}
}

// Figure out what to do :-)
if (isset($_GET['action']) || isset($_GET['search_id']))
{
	$action = (isset($_GET['action'])) ? $_GET['action'] : null;
	$sort_dir = (isset($_GET['sort_dir'])) ? (($_GET['sort_dir'] == 'DESC') ? 'DESC' : 'ASC') : 'DESC';

	// If a search_id was supplied
	if (isset($_GET['search_id']))
	{
		$search_id = intval($_GET['search_id']);
		if ($search_id < 1)
			message($lang_common['Bad request']);
	}
	// If it's a regular search (keywords and/or author)
	else if ($action == 'search')
	{
		$keywords = (isset($_GET['keywords'])) ? strtolower(trim($_GET['keywords'])) : null;
		$author = (isset($_GET['author'])) ? strtolower(trim($_GET['author'])) : null;
		$source = (isset($_GET['source'])) ? strtolower(trim($_GET['source'])) : null;
		$destination = (isset($_GET['destination'])) ? strtolower(trim($_GET['destination'])) : null;
		$description = (isset($_GET['destination'])) ? strtolower(trim($_GET['destination'])) : null;

		if (preg_match('#^[\*%]+$#', $keywords) || strlen(str_replace(array('*', '%'), '', $keywords)) < 3)
			$keywords = '';

		if (preg_match('#^[\*%]+$#', $author) || strlen(str_replace(array('*', '%'), '', $author)) < 3)
			$author = '';

		if (!$keywords && !$author && !$source && !$description && !$destination)
			message($lang_search['No terms']);

		if ($source)
			$source = str_replace('*', '%', $source);

		if ($destination)
			$destination = str_replace('*', '%', $destination);

		if ($author)
			$author = str_replace('*', '%', $author);

		if ($description)
			$description = '%'.$description.'%';


		$sort_by = (isset($_GET['sort_by'])) ? intval($_GET['sort_by']) : null;
	}
	else
			message($lang_common['Bad request']);


	// If a valid search_id was supplied we attempt to fetch the search results from the db
	if (isset($search_id))
	{
		$ident = ($pun_user['is_guest']) ? get_remote_address() : $pun_user['username'];

		$result = $db->query('SELECT search_data FROM '.$db->prefix.'search_cache WHERE id='.$search_id.' AND ident=\''.$db->escape($ident).'\'') or error('Unable to fetch search results', __FILE__, __LINE__, $db->error());
		if ($row = $db->fetch_assoc($result))
		{
			$temp = unserialize($row['search_data']);

			$search_results = $temp['search_results'];
			$num_hits = $temp['num_hits'];
			$sort_by = $temp['sort_by'];
			$sort_dir = $temp['sort_dir'];

			unset($temp);
		}
		else
		{
			if (!defined('PUN_HEADER'))
			{
				global $pun_user;
		
				$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / '.$lang_common['Info'];
				require PUN_ROOT.'header.php';
			}
?>
<div id="msg" class="block">
	<h2><span><?php echo $lang_common['Info'] ?></span></h2>
	<div class="box">
		<div class="inbox">
		<p><?php echo $lang_search['No hits'] ?></p>
<?php if (!$no_back_link): ?>		<p><a href="javascript: history.go(-1)"><?php echo $lang_common['Go back'] ?></a></p>
<?php endif; ?>		</div>
	</div>
</div>
<?php
			show_newgloss_box();
			require PUN_ROOT.'footer.php';
		}
	}
	else
	{
		$keyword_results = $author_results = array();

		if (!empty($author) || !empty($keywords) || !empty($source) || !empty($destination) || !empty($description))
		{
			// If it's a search for author name (and that author name isn't Guest)
			if ($author && strcasecmp($author, 'Guest') && strcasecmp($author, $lang_common['Guest']))
			{
				switch ($db_type)
				{
					case 'pgsql':
						$result = $db->query('SELECT id FROM '.$db->prefix.'users WHERE username ILIKE \''.$db->escape($author).'\'') or error('Unable to fetch users', __FILE__, __LINE__, $db->error());
						break;

					default:
						$result = $db->query('SELECT id FROM '.$db->prefix.'users WHERE username LIKE \''.$db->escape($author).'\'') or error('Unable to fetch users', __FILE__, __LINE__, $db->error());
						break;
				}

				if ($db->num_rows($result))
				{
					$user_ids = '';
					while ($row = $db->fetch_row($result))
						$user_ids .= (($user_ids != '') ? ',' : '').$row[0];

					$result = $db->query('SELECT id FROM '.$db->prefix.'glossary_items WHERE user_id IN('.$user_ids.')') or error('Unable to fetch matched glossary item list', __FILE__, __LINE__, $db->error());

					$search_ids = array();
					while ($row = $db->fetch_row($result))
						$author_results[] = $row[0];

					$db->free_result($result);
				}
			}


			$search_ids = $author_results;

			$num_hits = count($search_ids);

			$where_conditions = array();
			if (!empty($source))
				$where_conditions[] = 'src LIKE \''.$db->escape($source).'\'';

			if (!empty($destination))
				$where_conditions[] = 'dst LIKE \''.$db->escape($destination).'\'';
				
			if (!empty($description))
				$where_conditions[] = 'description LIKE \''.$db->escape($description).'\'';
				
			if (count($where_conditions))
				$where_conditions = implode(' AND ',$where_conditions);
			else
				$where_conditions = '';

			if ($num_hits)
			{
				if ($while_conditions != '')
					$while_conditions = ' AND '.$while_conditions;
				$result = $db->query('SELECT DISTINCT id FROM '.$db->prefix.'glossary_items WHERE id IN('.implode(',', $search_ids).') '.$where_conditions, true) or error('Unable to fetch topic list', __FILE__, __LINE__, $db->error());
			}
			else
				$result = $db->query('SELECT DISTINCT id FROM '.$db->prefix.'glossary_items WHERE '.$where_conditions, true) or error('Unable to fetch topic list', __FILE__, __LINE__, $db->error());

			$search_ids = array();
			while ($row = $db->fetch_row($result))
				$search_ids[] = $row[0];

			$db->free_result($result);

			$num_hits = count($search_ids);
		}
		else
			message($lang_common['Bad request']);


		// Prune "old" search results
		$result = $db->query('SELECT ident FROM '.$db->prefix.'online') or error('Unable to fetch online list', __FILE__, __LINE__, $db->error());

		if ($db->num_rows($result))
		{
			while ($row = $db->fetch_row($result))
				$old_searches[] = '\''.$db->escape($row[0]).'\'';

			$db->query('DELETE FROM '.$db->prefix.'search_cache WHERE ident NOT IN('.implode(',', $old_searches).')') or error('Unable to delete search results', __FILE__, __LINE__, $db->error());
		}

		// Final search results
		$search_results = implode(',', $search_ids);

		// Fill an array with our results and search properties
		$temp['search_results'] = $search_results;
		$temp['num_hits'] = $num_hits;
		$temp['sort_by'] = $sort_by;
		$temp['sort_dir'] = $sort_dir;
		$temp = serialize($temp);
		$search_id = mt_rand(1, 2147483647);

		$ident = ($pun_user['is_guest']) ? get_remote_address() : $pun_user['username'];

		$db->query('INSERT INTO '.$db->prefix.'search_cache (id, ident, search_data) VALUES('.$search_id.', \''.$db->escape($ident).'\', \''.$db->escape($temp).'\')') or error('Unable to insert search results', __FILE__, __LINE__, $db->error());

		if ($action != 'show_new' && $action != 'show_24h')
		{
			$db->end_transaction();
			$db->close();

			// Redirect the user to the cached result page
			header('Location: glossary.php?search_id='.$search_id);
			exit;
		}
	}


	// Fetch results to display
	if ($search_results != '')
	{
		$group_by_sql = '';
		switch ($sort_by)
		{
			case 1:
				$sort_by_sql = 'g.user_id';
				break;

			case 2:
				$sort_by_sql = 'g.src';
				break;

			case 3:
				$sort_by_sql = 'g.dst';
				break;

			case 4:
				$sort_by_sql = 'g.mtime';
				break;

			default:
			{
				$sort_by_sql = 'g.ctime';

				break;
			}
		}

		$substr_sql = ($db_type != 'sqlite') ? 'SUBSTRING' : 'SUBSTR';
		$sql = 'SELECT g.id, g.user_id, g.username, g.src, g.dst, g.ctime, g.mtime, g.topic_id, '.$substr_sql.'(g.description, 1, 1000) AS description FROM '.$db->prefix.'glossary_items AS g WHERE g.id IN('.$search_results.') ORDER BY '.$sort_by_sql;


		// Determine the topic or post offset (based on $_GET['p'])
		$per_page = $pun_user['disp_posts'];
		$num_pages = ceil($num_hits / $per_page);

		$p = (!isset($_GET['p']) || $_GET['p'] <= 1 || $_GET['p'] > $num_pages) ? 1 : $_GET['p'];
		$start_from = $per_page * ($p - 1);

		// Generate paging links
		$paging_links = $lang_common['Pages'].': '.paginate($num_pages, $p, 'glossary.php?search_id='.$search_id);


		$sql .= ' '.$sort_dir.' LIMIT '.$start_from.', '.$per_page;

		$result = $db->query($sql) or error('Unable to fetch search results', __FILE__, __LINE__, $db->error());

		$search_set = array();
		while ($row = $db->fetch_assoc($result))
			$search_set[] = $row;

		$db->free_result($result);

		$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / '.$lang_search['Search results'];
		require PUN_ROOT.'header.php';
?>
<div class="linkst">
	<div class="inbox">
		<p class="pagelink"><?php echo $paging_links ?></p>
	</div>
</div>

<?php
	foreach ($search_set as $cur_gloss) {
		show_gloss_item($cur_gloss);
?>
<!--
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
-->
<?php
}
?>

<div class="<?php echo 'linksb'; ?>">
	<div class="inbox">
		<p class="pagelink"><?php echo $paging_links ?></p>
	</div>
</div>
<?php

		show_newgloss_box();
		$footer_style = 'search';
		require PUN_ROOT.'footer.php';
	}
	else
	{
		if (!defined('PUN_HEADER'))
		{
			global $pun_user;
	
			$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / '.$lang_common['Info'];
			require PUN_ROOT.'header.php';
		}

?>
<div id="msg" class="block">
	<h2><span><?php echo $lang_common['Info'] ?></span></h2>
	<div class="box">
		<div class="inbox">
		<p><?php echo $lang_search['No hits'] ?></p>
<?php if (!$no_back_link): ?>		<p><a href="javascript: history.go(-1)"><?php echo $lang_common['Go back'] ?></a></p>
<?php endif; ?>		</div>
	</div>
</div>
<?php
		show_newgloss_box();
		require PUN_ROOT.'footer.php';
	}
}


$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / '.$lang_search['Search'];
$focus_element = array('search', 'keywords');
require PUN_ROOT.'header.php';

?>
<div id="searchform" class="blockform">
	<h2><span><?php echo $lang_search['Search'] ?></span></h2>
	<div class="box">
		<form id="search" method="get" action="glossary.php">
			<div class="inform">
				<fieldset>
					<legend><?php echo $lang_search['Search criteria legend'] ?></legend>
					<div class="infldset">
						<input type="hidden" name="action" value="search" />
						<label><?php echo '__Source' ?> <input type="text" name="source" size="40" maxlength="100" /><br /></label>
						<label><?php echo '__Translation' ?> <input type="text" name="destination" size="25" maxlength="25" /><br /></label>
						<label><?php echo $lang_search['Keyword search'] ?> <input type="text" name="keywords" size="40" maxlength="100" /><br /></label>
						<label><?php echo $lang_search['Author search'] ?> <input id="author" type="text" name="author" size="25" maxlength="25" /><br /></label>
						<label><?php echo '__description' ?> <input type="text" name="description" size="25" maxlength="25" /><br /></label>
						<p class="clearb"><?php echo $lang_search['Search info'] ?></p>
					</div>
				</fieldset>
			</div>
			<div class="inform">
				<fieldset>
					<legend><?php echo $lang_search['Search results legend'] ?></legend>
					<div class="infldset">
						<label class="conl"><?php echo $lang_search['Sort by'] ?>
						<br /><select name="sort_by">
							<option value="0"><?php echo $lang_search['Sort by post time'] ?></option>
							<option value="1"><?php echo $lang_search['Sort by author'] ?></option>
							<option value="2"><?php echo $lang_search['Sort by subject'] ?></option>
							<option value="3"><?php echo $lang_search['Sort by forum'] ?></option>
						</select>
						<br /></label>
						<label class="conl"><?php echo $lang_search['Sort order'] ?>
						<br /><select name="sort_dir">
							<option value="DESC"><?php echo $lang_search['Descending'] ?></option>
							<option value="ASC"><?php echo $lang_search['Ascending'] ?></option>
						</select>
						<br /></label>
						<p class="clearb"><?php echo $lang_search['Search results info'] ?></p>
					</div>
				</fieldset>
			</div>
			<p><input type="submit" name="search" value="<?php echo $lang_common['Submit'] ?>" accesskey="s" /></p>
		</form>
	</div>
</div>

<?php

require PUN_ROOT.'footer.php';

