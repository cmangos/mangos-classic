ALTER TABLE db_version CHANGE COLUMN required_z2712_01_mangos_creature_movement_drops required_z2713_01_mangos_creature_template_leashing bit;

ALTER TABLE creature_template ADD COLUMN `Detection` INT(10) UNSIGNED NOT NULL DEFAULT '20' COMMENT 'Detection range for proximity' AFTER `SpeedRun`;
ALTER TABLE creature_template ADD COLUMN `CallForHelp` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Range in which creature calls for help?' AFTER `Detection`;
ALTER TABLE creature_template ADD COLUMN `Pursuit` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'When exceeded during pursuit creature evades?' AFTER `CallForHelp`;
ALTER TABLE creature_template ADD COLUMN `Leash` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Leash range from combat start position' AFTER `Pursuit`;
ALTER TABLE creature_template ADD COLUMN `Timeout` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Time for refreshing leashing before evade?' AFTER `Leash`;


