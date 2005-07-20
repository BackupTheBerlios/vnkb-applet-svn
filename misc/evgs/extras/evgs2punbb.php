<?php
define('PUN_ROOT','./');
include PUN_ROOT.'include/common.php';
$result = $db->query("SELECT * FROM evgs_data");
$now = time();
while ($e = $db->fetch_assoc($result)) {
  $sql[] = 'INSERT INTO '.$db->prefix.'glossary_items (src,dst,user_id,username,ctime,mtime,description) VALUES(\''.$db->escape($e['term_en']).'\',\''.$db->escape($e['term_vi']).'\',2,\'admin\','.$now.','.$now.',\''.$db->escape($e['note']).'\')';
}
foreach ($sql as $s) {
// Uncomment this line to do the real work
//  if (!$db->query($s)) echo $db->error();
  echo "$s<br/>\n";
}
?>
