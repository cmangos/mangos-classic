ALTER TABLE character_db_version CHANGE COLUMN required_z1142_s0531_01_characters_bugreport required_z1217_s0636_02_characters_game_event_status bit;

DROP TABLE IF EXISTS `game_event_status`;
CREATE TABLE `game_event_status` (
  `event` smallint(6) unsigned NOT NULL default '0',
  PRIMARY KEY  (`event`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Game event system';
