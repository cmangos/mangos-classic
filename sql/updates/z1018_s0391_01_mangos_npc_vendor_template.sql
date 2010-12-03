ALTER TABLE db_version CHANGE COLUMN required_z1011_s0382_01_mangos_game_event_quest required_z1018_s0391_01_mangos_npc_vendor_template bit;

DROP TABLE IF EXISTS `npc_vendor_template`;
CREATE TABLE `npc_vendor_template` (
  `entry` mediumint(8) unsigned NOT NULL default '0',
  `item` mediumint(8) unsigned NOT NULL default '0',
  `maxcount` tinyint(3) unsigned NOT NULL default '0',
  `incrtime` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Npc System';
