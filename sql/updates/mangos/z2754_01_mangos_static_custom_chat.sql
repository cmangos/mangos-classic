ALTER TABLE db_version CHANGE COLUMN required_z2751_01_mangos_new_ticket_system required_z2754_01_mangos_static_custom_chat bit;

LOCK TABLES `command` WRITE;
/*!40000 ALTER TABLE `command` DISABLE KEYS */;
DELETE FROM `command` WHERE `name` = 'chat static';
INSERT INTO `command` VALUES
('chat static',2,'Syntax: .chat static $channelname on|off\r\n\r\nEnable or disable static mode for a custom channel with name $channelname. Static custom channel upon conversion acquires a set of properties identical to global channes.');
/*!40000 ALTER TABLE `command` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `mangos_string` WRITE;
/*!40000 ALTER TABLE `mangos_string` DISABLE KEYS */;
DELETE FROM `mangos_string` WHERE `entry` IN (373, 374, 375);
INSERT INTO `mangos_string` VALUES
(373,'Static custom chat: channel \'%s\' has password set.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(374,'Static custom chat: channel \'%s\' is not eligible for conversion.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(375,'Static custom chat: channel \'%s\', new status: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `mangos_string` ENABLE KEYS */;
UNLOCK TABLES;
