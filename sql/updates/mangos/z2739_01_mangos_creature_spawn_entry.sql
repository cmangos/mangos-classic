ALTER TABLE db_version CHANGE COLUMN required_z2738_01_mangos_quest_template required_z2739_01_mangos_creature_spawn_entry bit;

DROP TABLE IF EXISTS `creature_spawn_entry`;
CREATE TABLE `creature_spawn_entry` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Spawn Entry)';

