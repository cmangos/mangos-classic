ALTER TABLE db_version CHANGE COLUMN required_z0627_130_01_mangos_command required_z0630_131_01_mangos_game_event_pool bit;

DROP TABLE IF EXISTS `game_event_pool`;
