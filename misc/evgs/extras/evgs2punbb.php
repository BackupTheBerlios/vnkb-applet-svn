<?php
define('PUN_ROOT','./');
include PUN_ROOT.'include/common.php';
$result = $db->query("SELECT * FROM evgs_data");
$now = time();
/*
while ($e = $db->fetch_assoc($result)) {
  $sql[] = 'INSERT INTO '.$db->prefix.'glossary_items (src,dst,user_id,username,ctime,mtime,description) VALUES(\''.$db->escape($e['term_en']).'\',\''.$db->escape($e['term_vi']).'\',2,\'admin\','.$now.','.$now.',\''.$db->escape($e['note']).'\')';
}
*/
echo "<?php\n";
?>
define('PUN_ROOT','./');
include PUN_ROOT.'include/common.php';
$now = time();
<?php
// Uncomment this line to do the real work
//  if (!$db->query($s)) echo $db->error();
while ($e = $db->fetch_assoc($result)) {
?>
$db->query('INSERT INTO '.$db->prefix.'glossary_items (src,dst,user_id,username,ctime,mtime,description) VALUES(\''.$db->escape('<?php echo str2php($e['term_en'])?>').'\',\''.$db->escape('<?php echo str2php($e['term_vi'])?>').'\',2,\'admin\','.$now.','.$now.',\''.$db->escape('<?php echo str2php($e['note'])?>').'\')');
<?php
}
function str2php($str)
{
	return addslashes($str);
	$str = str_replace("'","\\'",$str);
	$str = str_replace("\n","\\n",$str);
	$str = str_replace("\r","\\r",$str);
	return $str;
}
?>
