ALTER TABLE db_version CHANGE COLUMN required_z2709_01_mangos_comments required_z2710_01_mangos_questgiver_greeting bit;

DROP TABLE IF EXISTS `questgiver_greeting`;
CREATE TABLE `questgiver_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Questgiver',
   `Type` INT(11) UNSIGNED NOT NULL COMMENT 'Type of entry',
   `Text` LONGTEXT COMMENT 'Text of the greeting',
   `EmoteId` INT(11) UNSIGNED NOT NULL COMMENT 'Emote ID of greeting',
   `EmoteDelay` INT(11) UNSIGNED NOT NULL COMMENT 'Emote delay of the greeting',
   PRIMARY KEY(`Entry`,`Type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Quest and Gossip system';

DROP TABLE IF EXISTS `locales_questgiver_greeting`;
CREATE TABLE `locales_questgiver_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Questgiver',
   `Type` INT(11) UNSIGNED NOT NULL COMMENT 'Type of entry',
   `Text_loc1` LONGTEXT COMMENT 'Text of the greeting locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the greeting locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the greeting locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the greeting locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the greeting locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the greeting locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the greeting locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the greeting locale 8',
   PRIMARY KEY(`Entry`,`Type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Quest and Gossip system';


