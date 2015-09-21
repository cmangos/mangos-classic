ALTER TABLE character_db_version CHANGE COLUMN required_z0876_s0201_01_characters_characters required_z0889_s0224_01_characters_creature_respawn bit;

DROP TABLE IF EXISTS `creature_respawn`;
CREATE TABLE `creature_respawn` (
  `guid` int(10) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `respawntime` bigint(20) NOT NULL default '0',
  `instance` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Grid Loading System';
