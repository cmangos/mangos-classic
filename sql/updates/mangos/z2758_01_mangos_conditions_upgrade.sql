ALTER TABLE db_version CHANGE COLUMN required_z2757_01_mangos_channel_commands required_z2758_01_mangos_conditions_upgrade bit;

ALTER TABLE `conditions`
	ADD COLUMN `value3` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'data field three for the condition' AFTER `value2`,
	ADD COLUMN `value4` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'data field four for the condition' AFTER `value3`,
	ADD COLUMN `flags` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `value4`,
	DROP INDEX `unique_conditions`,
	ADD UNIQUE INDEX `unique_conditions` (`type`, `value1`, `value2`, `value3`, `value4`, `flags`);

ALTER TABLE `dbscripts_on_creature_death`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_creature_movement`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_event`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_gossip`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_go_template_use`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_go_use`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_quest_end`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_quest_start`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_relay`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

ALTER TABLE `dbscripts_on_spell`
	ADD COLUMN `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER `o`;

