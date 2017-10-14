ALTER TABLE db_version CHANGE COLUMN required_z2713_01_mangos_creature_template_leashing required_z2714_01_mangos_model_speeds bit;

ALTER TABLE creature_model_info ADD COLUMN `SpeedWalk` FLOAT NOT NULL DEFAULT '1' COMMENT 'Default walking speed for any creature with model' AFTER `combat_reach`;
ALTER TABLE creature_model_info ADD COLUMN `SpeedRun` FLOAT NOT NULL DEFAULT '1.14286' COMMENT 'Default running speed for any creature with model' AFTER `SpeedWalk`;

ALTER TABLE creature_template MODIFY COLUMN `SpeedWalk` FLOAT NOT NULL DEFAULT '0';
ALTER TABLE creature_template MODIFY COLUMN `SpeedRun` FLOAT NOT NULL DEFAULT '0';


