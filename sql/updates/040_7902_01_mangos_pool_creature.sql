ALTER TABLE db_version CHANGE COLUMN required_039_7896_01_mangos_creature_template required_040_7902_01_mangos_pool_creature bit;

ALTER TABLE `pool_creature`
   ADD INDEX `idx_guid`(`guid`);