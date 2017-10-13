ALTER TABLE db_version CHANGE COLUMN required_z2710_01_mangos_questgiver_greeting required_z2711_01_mangos_area_trigger_locales bit;

ALTER TABLE areatrigger_teleport ADD COLUMN status_failed_text TEXT AFTER target_orientation;

DROP TABLE IF EXISTS `locales_areatrigger_teleport`;
CREATE TABLE `locales_areatrigger_teleport` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Areatrigger Teleport',
   `Text_loc1` LONGTEXT COMMENT 'Text of the status_failed locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the status_failed locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the status_failed locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the status_failed locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the status_failed locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the status_failed locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the status_failed locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the status_failed locale 8',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Areatrigger System';


