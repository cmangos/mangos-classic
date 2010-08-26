ALTER TABLE db_version CHANGE COLUMN required_z0803_s0116_01_mangos_creature_movement_template required_z0817_xxxxx_01_mangos_game_event bit;

ALTER TABLE game_event 
  ADD COLUMN holiday mediumint(8) unsigned NOT NULL default '0' COMMENT 'Client side holiday id' AFTER length;

