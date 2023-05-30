ALTER TABLE db_version CHANGE COLUMN required_z2807_01_mangos_addon_rename required_z2808_01_mangos_string_id bit;

DROP TABLE IF EXISTS `string_id`;
CREATE TABLE `string_id`(
`Id` INT(11) UNSIGNED NOT NULL,
`Name` VARCHAR(255) NOT NULL,
PRIMARY KEY(`Id`)
);

ALTER TABLE creature_spawn_data_template ADD COLUMN `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE gameobject_addon ADD COLUMN `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE spawn_group ADD COLUMN `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE creature_template ADD COLUMN `StringId1` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `SpellList`;
ALTER TABLE creature_template ADD COLUMN `StringId2` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `StringId1`;
ALTER TABLE gameobject_template ADD COLUMN `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `maxgold`;
ALTER TABLE `creature_spawn_data_template` ADD COLUMN `Name` VARCHAR(200) NOT NULL;
UPDATE creature_spawn_data_template SET Name='GENERIC - NONE - DO NOT EDIT' WHERE entry IN(0);
UPDATE creature_spawn_data_template SET Name='GENERIC - SPAWN FLAG RUN' WHERE entry IN(1);
UPDATE creature_spawn_data_template SET Name='GENERIC - SPAWN FLAG HOVER' WHERE entry IN(2);
UPDATE creature_spawn_data_template SET Name='GENERIC - SPAWN FLAG RUN + HOVER' WHERE entry IN(3);

