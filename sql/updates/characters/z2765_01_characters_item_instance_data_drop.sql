ALTER TABLE character_db_version CHANGE COLUMN required_z2764_01_characters_ahbot_items required_z2765_01_characters_item_instance_data_drop bit;

-- WARNING: It's recommended you take a BACKUP of your entire character database BEFORE running this update.

CREATE TABLE item_instance_backup_pre_data_field_drop AS (SELECT * FROM item_instance);

ALTER TABLE `item_instance`
 ADD `itemEntry` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `owner_guid`,
 ADD `creatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `itemEntry`,
 ADD `giftCreatorGuid` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `creatorGuid`,
 ADD `count` INT(10) UNSIGNED NOT NULL DEFAULT '1' AFTER `giftCreatorGuid`,
 ADD `duration` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `count`,
 ADD `charges` TEXT NOT NULL AFTER `duration`,
 ADD `flags` INT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `charges`,
 ADD `enchantments` TEXT NOT NULL AFTER `flags`,
 ADD `randomPropertyId` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `enchantments`,
 ADD `durability` INT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `randomPropertyId`,
 ADD `itemTextId` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `durability`;
 
 -- Temporarily change delimiter to prevent SQL syntax errors
DELIMITER ||

-- Function to convert ints from unsigned to signed
DROP FUNCTION IF EXISTS `uint32toint32`||
CREATE FUNCTION `uint32toint32`(input INT(10) UNSIGNED) RETURNS BIGINT(20) SIGNED DETERMINISTIC
BEGIN
  RETURN CAST((input<<32) AS SIGNED)/(1<<32);
END||

-- Restore original delimiter
DELIMITER ;

-- Move data to new fields
UPDATE `item_instance` SET
`itemEntry` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',3))+2, length(SUBSTRING_INDEX(`data`,' ',3+1))-length(SUBSTRING_INDEX(data,' ',3))-1),
`creatorGuid` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',10))+2, length(SUBSTRING_INDEX(`data`,' ',10+1))-length(SUBSTRING_INDEX(data,' ',10))-1),
`giftCreatorGuid` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',12))+2, length(SUBSTRING_INDEX(`data`,' ',12+1))-length(SUBSTRING_INDEX(data,' ',12))-1),
`count` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',14))+2, length(SUBSTRING_INDEX(`data`,' ',14+1))-length(SUBSTRING_INDEX(data,' ',14))-1),
`duration` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',15))+2, length(SUBSTRING_INDEX(`data`,' ',15+1))-length(SUBSTRING_INDEX(data,' ',15))-1),
`charges` = CONCAT_WS(' ',
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',16))+2, length(SUBSTRING_INDEX(`data`,' ',16+1))-length(SUBSTRING_INDEX(data,' ',16))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',17))+2, length(SUBSTRING_INDEX(`data`,' ',17+1))-length(SUBSTRING_INDEX(data,' ',17))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',18))+2, length(SUBSTRING_INDEX(`data`,' ',18+1))-length(SUBSTRING_INDEX(data,' ',18))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',19))+2, length(SUBSTRING_INDEX(`data`,' ',19+1))-length(SUBSTRING_INDEX(data,' ',19))-1)),
 uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',20))+2, length(SUBSTRING_INDEX(`data`,' ',20+1))-length(SUBSTRING_INDEX(data,' ',20))-1)) ),
`flags` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',21))+2, length(SUBSTRING_INDEX(`data`,' ',21+1))-length(SUBSTRING_INDEX(data,' ',21))-1),
`enchantments` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',22))+2, length(SUBSTRING_INDEX(`data`,' ',42+1))-length(SUBSTRING_INDEX(data,' ',22))-1),
`randomPropertyId` = uint32toint32(SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',44))+2, length(SUBSTRING_INDEX(`data`,' ',44+1))-length(SUBSTRING_INDEX(data,' ',44))-1)),
`durability` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',46))+2, length(SUBSTRING_INDEX(`data`,' ',46+1))-length(SUBSTRING_INDEX(data,' ',46))-1),
`itemTextId` = SUBSTRING(`data`, length(SUBSTRING_INDEX(`data`,' ',45))+2, length(SUBSTRING_INDEX(`data`,' ',45+1))-length(SUBSTRING_INDEX(data,' ',45))-1);

-- Drop function
DROP FUNCTION IF EXISTS `uint32toint32`;

-- Drop old field 
 ALTER TABLE `item_instance` DROP `data`;
