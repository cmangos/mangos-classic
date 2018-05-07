ALTER TABLE db_version CHANGE COLUMN required_z2718_01_mangos_spell_affect required_z2719_01_mangos_taxi_system_update bit;

DELETE FROM `command` WHERE `name` = 'debug taxi';
INSERT INTO `command` VALUES
('debug taxi',3,'Syntax: .debug taxi\r\n\r\nToggle debug mode for taxi flights. In debug mode GM receive additional on-screen information during taxi flights.');

DELETE FROM `mangos_string` WHERE `entry` IN (1196, 1197, 1198, 1199);
INSERT INTO `mangos_string` VALUES
(1196,'Debug output for taxi flights is now %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1197,'[Taxi]: Progress at node: [%u][%u], next: [%u][%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1198,'[Taxi]: Progress at final node: [%u][%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1199,'[Taxi]: Changing route to [%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

DROP TABLE IF EXISTS `taxi_shortcuts`;
CREATE TABLE `taxi_shortcuts` (
  `pathid` int unsigned NOT NULL COMMENT 'Flight path entry id',
  `takeoff` int unsigned NOT NULL COMMENT 'Amount of waypoints to skip in the beginning of the flight',
  `landing` int unsigned NOT NULL COMMENT 'Amount of waypoints to skip at the end of the flight',
  `comments` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`pathid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Taxi System';

