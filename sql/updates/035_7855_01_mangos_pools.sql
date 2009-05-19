ALTER TABLE db_version CHANGE COLUMN required_029_7393_01_mangos_game_event required_035_7855_01_mangos_pools bit;

ALTER TABLE pool_creature
  ADD COLUMN description varchar(255) NOT NULL AFTER chance;

ALTER TABLE pool_gameobject
  ADD COLUMN description varchar(255) NOT NULL AFTER chance;

ALTER TABLE pool_pool
  ADD COLUMN description varchar(255) NOT NULL AFTER chance;

ALTER TABLE pool_template
  ADD COLUMN description varchar(255) NOT NULL AFTER max_limit;
