ALTER TABLE db_version CHANGE COLUMN required_z2829_01_mangos_proc_cooldown required_z2830_01_mangos_icon_name bit;

ALTER TABLE `gameobject_template` ADD COLUMN `IconName` varchar(100) NOT NULL DEFAULT '' AFTER `name`;