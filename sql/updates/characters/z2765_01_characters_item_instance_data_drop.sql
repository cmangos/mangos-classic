ALTER TABLE character_db_version CHANGE COLUMN required_z2764_01_characters_ahbot_items required_z2765_01_characters_item_instance_data_drop bit;

-- WARNING: You MUST use iteminstance_converter script in contrib folder to preserve character item data.
-- Follow the README instructions located in the tool's folder.
-- It's recommended you take a BACKUP of your entire character database BEFORE running this update.

CREATE TABLE item_instance_backup_pre_data_field_drop AS (SELECT * FROM item_instance);

TRUNCATE `item_instance`;

ALTER TABLE `item_instance` DROP `data`;

ALTER TABLE `item_instance`
 ADD `itemEntry` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `owner_guid`,
 ADD `creatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `itemEntry`,
 ADD `giftCreatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `creatorGuid`,
 ADD `count` INT(10) UNSIGNED NOT NULL DEFAULT '1' AFTER `giftCreatorGuid`,
 ADD `duration` INT(10) UNSIGNED NOT NULL AFTER `count`,
 ADD `charges` TEXT NOT NULL AFTER `duration`,
 ADD `flags` INT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `charges`,
 ADD `enchantments` TEXT NOT NULL AFTER `flags`,
 ADD `randomPropertyId` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `enchantments`,
 ADD `durability` INT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `randomPropertyId`,
 ADD `itemTextId` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `durability`;