ALTER TABLE db_version CHANGE COLUMN required_z2742_01_mangos_visibility required_z2745_01_mangos_game_event_split bit;

DROP TABLE IF EXISTS `game_event_time`;
CREATE TABLE `game_event_time` (
  `entry` mediumint(8) unsigned NOT NULL COMMENT 'Entry of the game event',
  `start_time` DATETIME NOT NULL DEFAULT '1970-01-01 00:00:00' COMMENT 'Absolute start date, the event will never start before',
  `end_time` DATETIME NOT NULL DEFAULT '1970-01-01 00:00:00' COMMENT 'Absolute end date, the event will never start after',
    PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

ALTER TABLE `game_event` ADD COLUMN `schedule_type` INT(11) NOT NULL DEFAULT 0 AFTER `entry`;
UPDATE `game_event` SET `schedule_type`=1;

INSERT INTO `game_event_time`(`entry`, `start_time`, `end_time`) SELECT `entry`, `start_time`, `end_time` FROM `game_event` WHERE `schedule_type` = 1;

ALTER TABLE `game_event` DROP COLUMN `start_time`;
ALTER TABLE `game_event` DROP COLUMN `end_time`;


