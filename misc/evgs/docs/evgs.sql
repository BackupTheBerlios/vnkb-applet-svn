CREATE TABLE `evgs_glossary_items` (
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
) TYPE=MyISAM;

CREATE TABLE `evgs_glossrev_items` (
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
) TYPE=MyISAM;

-- INSERT INTO `evgs_config` VALUES ('o_evgs_forum', '1');
