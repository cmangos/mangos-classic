CREATE DATABASE `classiclogs` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;

GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES ON `classiclogs`.* TO 'mangos'@'localhost';

USE `classiclogs`;

DROP TABLE IF EXISTS `logs_db_version`;
CREATE TABLE `logs_db_version` (
  `required_z2778_01_logs_anticheat` bit(1) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Last applied sql update to DB';

LOCK TABLES `logs_db_version` WRITE;
/*!40000 ALTER TABLE `logs_db_version` DISABLE KEYS */;
INSERT INTO `logs_db_version` VALUES
(NULL);
/*!40000 ALTER TABLE `logs_db_version` ENABLE KEYS */;
UNLOCK TABLES;

# Logs database

DROP TABLE IF EXISTS `logs_anticheat`;

CREATE TABLE `logs_anticheat` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `realm` INT(10) UNSIGNED NOT NULL,
  `account` INT(10) UNSIGNED NOT NULL,
  `ip` VARCHAR(16) NOT NULL,
  `fingerprint` int(10) unsigned NOT NULL,
  `actionMask` INT(10) UNSIGNED DEFAULT NULL,
  `player` VARCHAR(32) NOT NULL,
  `info` VARCHAR(512) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `account` (`account`),
  KEY `ip` (`ip`),
  KEY `time` (`time`),
  KEY `realm` (`realm`)
) ENGINE=INNODB AUTO_INCREMENT=34 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `logs_spamdetect`;

CREATE TABLE `logs_spamdetect` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `realm` int(10) unsigned NOT NULL,
  `accountId` int(11) DEFAULT '0',
  `fromGuid` bigint unsigned DEFAULT '0',
  `fromIP` varchar(16) NOT NULL,
  `fromFingerprint` int(10) unsigned NOT NULL,
  `comment` varchar(8192) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `guid` (`fromGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


