ALTER TABLE db_version CHANGE COLUMN required_035_7855_01_mangos_pools required_029_7393_01_mangos_game_event bit;

ALTER TABLE pool_creature
  DROP COLUMN description;

ALTER TABLE pool_gameobject
  DROP COLUMN description;

ALTER TABLE pool_pool
  DROP COLUMN description;

ALTER TABLE pool_template
  DROP COLUMN description;
