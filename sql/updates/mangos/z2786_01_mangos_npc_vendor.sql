ALTER TABLE db_version CHANGE COLUMN required_z2785_01_mangos_waypoint_path z2786_01_mangos_npc_vendor bit;

ALTER TABLE `npc_vendor` ADD COLUMN `slot` TINYINT(3) UNSIGNED DEFAULT 0 NOT NULL AFTER `incrtime`;
ALTER TABLE `npc_vendor_template` ADD COLUMN `slot` TINYINT(3) UNSIGNED DEFAULT 0 NOT NULL AFTER `incrtime`;
