ALTER TABLE db_version CHANGE COLUMN required_029_7393_01_mangos_game_event required_020_7643_01_mangos_db_version bit;

ALTER TABLE game_event
  DROP COLUMN holiday;
