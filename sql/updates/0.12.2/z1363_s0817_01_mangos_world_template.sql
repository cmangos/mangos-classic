ALTER TABLE db_version CHANGE COLUMN required_z1357_s0811_01_mangos_spell_threat required_z1363_s0817_01_mangos_world_template bit;

--
-- Table structure for table `world_template`
--

DROP TABLE IF EXISTS `world_template`;
CREATE TABLE `world_template` (
  `map` smallint(5) unsigned NOT NULL,
  `ScriptName` varchar(128) NOT NULL default '',
  PRIMARY KEY  (`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
