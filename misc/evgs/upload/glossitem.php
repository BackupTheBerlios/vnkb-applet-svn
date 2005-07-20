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

$id = isset($_GET['id']) ? intval($_GET['id']) : null;
$action = isset($_GET['action']) ? $_GET['action'] : 'history';

if (!$id)
	message($lang_common['Bad request']);

$result = $db->query('SELECT * FROM '.$db->prefix.'glossary_items WHERE id='.$id) or error('Could not fetch glossary item info',__FILE__,__LINE__,$db->error());
if (!$db->num_rows($result))
	message($lang_common['Bad request']);
$cur_gloss = $db->fetch_assoc($result);

if (isset($_POST['action']))
{
	if ($_POST['action'] == 'editgloss')
	{
	$cur_gloss = array(
		'id' => $id,
		'src' => (isset($_POST['source'])) ? strtolower(trim($_POST['source'])) : null,
		'dst' => (isset($_POST['destination'])) ? strtolower(trim($_POST['destination'])) : null,
		'description' => (isset($_POST['description'])) ? strtolower(trim($_POST['description'])) : null
		);
		process_newgloss($cur_gloss,true);
	}
}

$page_title = 'Edit gloss';
include PUN_ROOT.'header.php';

switch ($action)
{
	case 'edit':
		if ($pun_user['is_guest'])
			message($lang_common['Bad request']);
		show_newgloss_box($cur_gloss,true);
?>

<?php

	default:
		$result = $db->query('SELECT * FROM '.$db->prefix.'glossary_items WHERE id='.$id) or error('Could not fetch glossary item info',__FILE__,__LINE__,$db->error());
		if (!$db->num_rows($result))
			message($lang_common['Bad request']);
		$cur_gloss = $db->fetch_assoc($result);
		show_gloss_item($cur_gloss);
		$result = $db->query('SELECT * FROM '.$db->prefix.'glossrev_items WHERE id='.$id.' ORDER by rev_id DESC') or error('Could not fetch glossary item info',__FILE__,__LINE__,$db->error());
		while ($cur_gloss = $db->fetch_assoc($result))
			show_gloss_item($cur_gloss,true);

}


include PUN_ROOT.'footer.php';


