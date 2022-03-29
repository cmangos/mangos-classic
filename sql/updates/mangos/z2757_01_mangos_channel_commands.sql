ALTER TABLE db_version CHANGE COLUMN required_z2756_01_mangos_new_ticket_system_update required_z2757_01_mangos_channel_commands bit;

LOCK TABLES `command` WRITE;
/*!40000 ALTER TABLE `command` DISABLE KEYS */;
DELETE FROM `command` WHERE `name` IN ('chat static', 'channel list', 'channel static');
INSERT INTO `command` VALUES
('channel list',1,'Syntax: .channel list [#max] [static]\r\n\r\nShow list of custom channels with amounts of players joined.'),
('channel static',1,'Syntax: .channel static $channelname on|off\r\n\r\nEnable or disable static mode for a custom channel with name $channelname. Static custom channel upon conversion acquires a set of properties identical to global channes.');
/*!40000 ALTER TABLE `command` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `mangos_string` WRITE;
/*!40000 ALTER TABLE `mangos_string` DISABLE KEYS */;
DELETE FROM `mangos_string` WHERE `entry` IN (176, 177, 178, 179, 180, 181, 182, 373, 374, 375);
INSERT INTO `mangos_string` VALUES
(176,'Conversion failed: channel \'%s\' has password set',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(177,'Conversion failed: channel \'%s\' is not eligible',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(178,'Conversion succeeded: channel \'%s\' static status is now %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(179,'(Static)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(180,'(Password)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(181,'There are no matching custom channels at the moment',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(182,'Listing up to %u custom channels matching criterea:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(373,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(374,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(375,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `mangos_string` ENABLE KEYS */;
UNLOCK TABLES;
