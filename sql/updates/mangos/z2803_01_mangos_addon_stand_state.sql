ALTER TABLE db_version CHANGE COLUMN required_z2802_01_mangos_reference_loot_template_names required_z2803_01_mangos_addon_stand_state bit;

ALTER TABLE `creature_template_addon` ADD COLUMN `stand_state` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `bytes1`;
ALTER TABLE `creature_addon` ADD COLUMN `stand_state` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `bytes1`;
UPDATE creature_template_addon SET stand_state = (bytes1 & 0xFF) WHERE bytes1 != 0;
UPDATE creature_addon SET stand_state = (bytes1 & 0xFF) WHERE bytes1 != 0;
ALTER TABLE creature_template_addon DROP COLUMN bytes1;
ALTER TABLE creature_addon DROP COLUMN bytes1;


