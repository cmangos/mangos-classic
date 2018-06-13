ALTER TABLE character_db_version CHANGE COLUMN required_z2719_01_characters_taxi_system_update required_z2720_01_characters_warden_system bit;

DROP TABLE IF EXISTS `warden_action`;
CREATE TABLE `warden_action` (
  `wardenId` SMALLINT(5) UNSIGNED NOT NULL,
  `action` TINYINT(3) UNSIGNED DEFAULT NULL,
  PRIMARY KEY (`wardenId`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;
