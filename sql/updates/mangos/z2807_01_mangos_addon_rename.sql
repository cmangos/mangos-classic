ALTER TABLE db_version CHANGE COLUMN required_z2806_01_mangos_creature_drops required_z2807_01_mangos_addon_rename bit;

ALTER TABLE creature_template_addon CHANGE b2_0_sheath sheath_state tinyint(3) unsigned NOT NULL DEFAULT '0';
ALTER TABLE creature_addon CHANGE b2_0_sheath sheath_state tinyint(3) unsigned NOT NULL DEFAULT '0';

ALTER TABLE `creature_spawn_data_template` CHANGE `EquipmentId` `EquipmentId` MEDIUMINT(8) NOT NULL DEFAULT '-1';


