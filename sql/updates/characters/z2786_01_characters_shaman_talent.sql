ALTER TABLE character_db_version CHANGE COLUMN required_z2775_01_characters_raf required_z2786_01_characters_shaman_talent bit;

DROP TABLE IF EXISTS `character_forgotten_skills`;
CREATE TABLE `character_forgotten_skills` (
  `guid` int(11) unsigned NOT NULL COMMENT 'Global Unique Identifier',
  `skill` mediumint(9) unsigned NOT NULL,
  `value` mediumint(9) unsigned NOT NULL,
  PRIMARY KEY (`guid`,`skill`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';


