ALTER TABLE db_version CHANGE COLUMN required_z2785_01_mangos_waypoint_path required_z2787_01_mangos_npc_vendor bit;

ALTER TABLE `npc_vendor` ADD COLUMN `slot` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `incrtime`;
ALTER TABLE `npc_vendor_template` ADD COLUMN `slot` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `incrtime`;
