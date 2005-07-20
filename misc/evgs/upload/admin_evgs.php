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


// Tell header.php to use the admin template
define('PUN_ADMIN_CONSOLE', 1);

define('PUN_ROOT', './');
require PUN_ROOT.'include/common.php';
require PUN_ROOT.'include/common_admin.php';


if ($pun_user['g_id'] > PUN_ADMIN)
	message($lang_common['No permission']);


if (isset($_POST['form_sent']))
{
	// Lazy referer check (in case base_url isn't correct)
	if (!isset($_SERVER['HTTP_REFERER']) || !preg_match('#/admin_evgs\.php#i', $_SERVER['HTTP_REFERER']))
		message($lang_common['Bad referrer']);

	$form = array_map('trim', $_POST['form']);


	$form['evgs_forum'] = intval($form['evgs_forum']);

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

	redirect('admin_evgs.php', 'Options updated. Redirecting &hellip;');
}


$page_title = pun_htmlspecialchars($pun_config['o_board_title']).' / Admin / Options';
$form_name = 'update_options';
require PUN_ROOT.'header.php';

generate_admin_menu('evgs');

?>
	<div class="blockform">
		<h2><span>Options</span></h2>
		<div class="box">
			<form method="post" action="admin_evgs.php?action=foo">
				<p class="submittop"><input type="submit" name="save" value="Save changes" /></p>
				<div class="inform">
				<input type="hidden" name="form_sent" value="1" />
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
