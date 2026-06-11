ALTER TABLE character_db_version CHANGE COLUMN required_z2819_01_characters_item_instance_text_id_fix required_z2820_01_characters_roguelike_level_bonus bit;

DROP TABLE IF EXISTS `character_roguelike_level_bonus`;
CREATE TABLE `character_roguelike_level_bonus` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `level` smallint(5) unsigned NOT NULL DEFAULT '0',
  `stat` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `value` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`, `level`, `stat`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Persistent roguelike stat bonuses rolled on level up';
