ALTER TABLE db_version CHANGE COLUMN required_z2722_01_mangos_creature_waypoint_refurbishing required_z2725_01_mangos_trainer_greeting bit;

DROP TABLE IF EXISTS `trainer_greeting`;
CREATE TABLE `trainer_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Trainer',
   `Text` LONGTEXT COMMENT 'Text of the greeting',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Trainer system';

DROP TABLE IF EXISTS `locales_trainer_greeting`;
CREATE TABLE `locales_trainer_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Trainer',
   `Text_loc1` LONGTEXT COMMENT 'Text of the greeting locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the greeting locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the greeting locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the greeting locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the greeting locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the greeting locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the greeting locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the greeting locale 8',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Trainer system';


