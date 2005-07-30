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


// Make sure no one attempts to run this script "directly"
if (!defined('PUN'))
	exit;

// Tell admin_loader.php that this is indeed a plugin and that it is loaded
define('PUN_PLUGIN_LOADED', 1);

if (isset($_POST['form_sent']))
{
	$form = array_map('trim', $_POST['form']);


	$form['evgs_forum'] = intval($form['evgs_forum']);
	$form['quick_links'] = intval($form['quick_links']);

	while (list($key, $input) = @each($form))
	{
		// Only update values that have changed
		if (isset($pun_config['o_'.$key]) && $pun_config['o_'.$key] != $input)
		{
			if ($input != '' || is_int($input))
				$value = '\''.$db->escape($input).'\'';
			else
				$value = 'NULL';

			$db->query('UPDATE '.$db->prefix.'config SET conf_value='.$value.' WHERE conf_name=\'o_'.$db->escape($key).'\'') or error('Unable to update board config', __FILE__, __LINE__, $db->error());
		}
	}

	// Regenerate the config cache
	require_once PUN_ROOT.'include/cache.php';
	generate_config_cache();

	redirect($_SERVER['REQUEST_URI'], 'Options updated. Redirecting &hellip;');
}

$func_check = array();

if (function_exists('evgs_config_check'))
{
	$sql_config_check = evgs_config_check();
	if (count($sql_config_check))
	{
		if (function_exists('evgs_config_repair'))
			$sql_config = evgs_config_repair();
		else
		{
			$sql_config = array();
			$func_check[] = 'Function evgs_config_repair not found.';
		}
	}
}
else
{
	$sql_config_check = array();
	$file_check = 'Function evgs_config_check not found.';
}

if (function_exists('evgs_table_check'))
{
	$sql_table_check = evgs_table_check();
	if (count($sql_table_check))
	{
		if (function_exists('evgs_table_repair'))
			$sql_table = evgs_table_repair();
		else
		{
			$sql_table = array();
			$func_check[] = 'Function evgs_table_repair not found.';
		}
	}
}
else
{
	$sql_table_check = array();
	$file_check = 'Function evgs_table_check not found.';
}

if (function_exists('evgs_file_check'))
{
	$sql_file_check = evgs_file_check();
	if (count($sql_file_check))
	{
		if (function_exists('evgs_file_repair'))
			$sql_file = evgs_file_repair();
		else
		{
			$sql_file = array();
			$func_check[] = 'Function evgs_file_repair not found.';
		}
	}
}
else
{
	$sql_file_check = array();
	$func_check[] = 'Function evgs_file_check not found.';
}

$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / Admin / EVGS';
$form_name = 'update_options';
require PUN_ROOT.'header.php';

generate_admin_menu($plugin);

?>

	<div class="blockform">
		<h2><span>Administration</span></h2>
		<div class="box">
			<form method="post" action="<?php echo $_SERVER['REQUEST_URI'] ?>&amp;foo=bar">
				<input type="hidden" name="form_sent" value="1" />
				<div class="inform">
					<fieldset>
						<legend>Status</legend>
						<div class="infldset">
							<table class="aligntop" cellspacing="0">
<?php if (count($func_check)): ?>
								<tr>
									<th scope="row">Fatal Errors</th>
									<td>
										<?php echo nl2br(pun_htmlspecialchars(join("\n",$func_check))); ?>
										<p>
											Please make sure <b>include/glossary.php</b> is included in <b>include/common.php</b>
										</p>
									</td>
								</tr>
<?php else: ?>
								<tr>
									<th scope="row">File Check</th>
									<td><?php echo count($sql_file_check) ? nl2br(pun_htmlspecialchars(join("\n",$sql_file_check))) : 'OK'; ?></td>
								</tr>
								<tr>
									<th scope="row">EVGS Configuration</th>
									<td><?php echo count($sql_config_check) ? nl2br(pun_htmlspecialchars(join("\n",$sql_config_check))) : 'OK'; ?></td>
								</tr>
								<tr>
									<th scope="row">EVGS Database</th>
									<td><?php echo count($sql_table_check) ? nl2br(pun_htmlspecialchars(join("\n",$sql_table_check))) : 'OK'; ?></td>
								</tr>
<?php endif; ?>
							</table>
					</fieldset>
<?php if (count($sql_config) || count($sql_table)): ?>
					<fieldset>
						<legend>SQL Repair</legend>
						<div class="infldset">
							<table class="aligntop" cellspacing="0">
<?php		foreach ($sql_config as $sql): ?>
								<tr>
									<th scope="row">EVGS Configuration</th>
									<td><?php echo nl2br(pun_htmlspecialchars($sql)); ?></td>
								</tr>
<?php		endforeach; ?>
<?php		foreach ($sql_table as $sql): ?>
								<tr>
									<th scope="row">EVGS Database</th>
									<td><?php echo nl2br(pun_htmlspecialchars($sql)); ?></td>
								</tr>
<?php		endforeach; ?>
							</table>
					</fieldset>
<?php endif; ?>
				</div>
				<!-- p class="submitend"><input type="submit" name="save" value="Install" /><input type="submit" name="save" value="Repair" /><input type="submit" name="save" value="Backup" /></p -->
			</form>
		</div>
	</div>

	<div class="blockform block2">
		<h2><span>Options</span></h2>
		<div class="box">
			<form method="post" action="<?php echo $_SERVER['REQUEST_URI'] ?>&amp;foo=bar">
				<p class="submittop"><input type="submit" name="save" value="Save changes" /></p>
				<input type="hidden" name="form_sent" value="1" />
				<div class="inform">
					<fieldset>
						<legend>Options</legend>
						<div class="infldset">
							<table class="aligntop" cellspacing="0">
								<tr>
									<th scope="row">Comment room</th>
									<td>
										<select name="form[evgs_forum]">
<?php
$result = $db->query('SELECT * FROM '.$db->prefix.'forums ORDER BY forum_name') or error('Could not fetch forum info',__FILE__,__LINE__,$db->error());
while ($room = $db->fetch_assoc($result)) {
?>
											<option value="<?php echo $room['id']?>"<?php if ($pun_config['o_evgs_forum'] == $room['id'] ) echo ' selected="selected"' ?>><?php echo pun_htmlspecialchars($room['forum_name'])?></option>
<?php
}
?>
										</select>
										<span>This room is used to store glossary comments.</span>
									</td>
								</tr>
								<tr>
									<th scope="row">Show abc band</th>
									<td>
										<input type="radio" name="form[quick_links]" value="1"<?php if ($pun_config['o_quick_links'] == '1') echo ' checked="checked"' ?> />&nbsp;<strong>Yes</strong>&nbsp;&nbsp;&nbsp;<input type="radio" name="form[quick_links]" value="0"<?php if ($pun_config['o_quick_links'] == '0') echo ' checked="checked"' ?> />&nbsp;<strong>No</strong>
										<span>Show alphabet band or not.</span>
									</td>
								</tr>
								<tr>
									<th scope="row">ABC Band Content</th>
									<td>
										<textarea name="form[quick_links_content]" rows="5" cols="55"><?php echo pun_htmlspecialchars($pun_config['o_quick_links_content']) ?></textarea>
										<span>This text will not be parsed like regular posts and thus may contain HTML.</span>
									</td>
								</tr>
							</table>
						</div>
					</fieldset>
				</div>
				<p class="submitend"><input type="submit" name="save" value="Save changes" /></p>
			</form>
		</div>
	</div>
	<div class="clearer"></div>
</div>
<?php

require PUN_ROOT.'footer.php';
