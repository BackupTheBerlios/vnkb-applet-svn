<?php
define ('PUN_ROOT','./');
define('PUN_QUIET_VISIT', 1);
include PUN_ROOT.'include/common.php';
$pattern = $_GET['id'];
$max = isset($_GET['max']) ? ' LIMIT '.intval($_GET['max']) : '';
$result = $db->query('SELECT DISTINCT src FROM '.$db->prefix.'glossary_items WHERE src LIKE \''.$db->escape($pattern).'%\' ORDER BY src '.$max);
$first_row = true;
while ($row = $db->fetch_row($result)) {
	if ($first_row)
		$first_row = false;
	else
		echo '||';
	echo $row[0].'|'.$row[0];
}
