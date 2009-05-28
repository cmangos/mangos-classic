ALTER TABLE db_version CHANGE COLUMN required_040_7902_02_mangos_pool_gameobject required_039_7896_01_mangos_creature_template  bit;

ALTER TABLE `pool_creature`
   DROP INDEX `idx_guid`;

ALTER TABLE `pool_gameobject`
   DROP INDEX `idx_guid`;