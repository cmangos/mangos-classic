ALTER TABLE db_version CHANGE COLUMN required_020_7643_01_mangos_db_version required_029_7393_01_mangos_game_event bit;

ALTER TABLE game_event
  ADD COLUMN holiday mediumint(8) unsigned NOT NULL default '0' COMMENT 'Client side holiday id' AFTER length;
