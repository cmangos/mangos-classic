ALTER TABLE db_version CHANGE COLUMN required_z2755_01_mangos_broadcast_text required_z2756_01_mangos_new_ticket_system_update bit;

LOCK TABLES `command` WRITE;
/*!40000 ALTER TABLE `command` DISABLE KEYS */;
DELETE FROM `command` WHERE `name` IN ('ticket', 'ticket whisper', 'tickets', 'tickets list');
INSERT INTO `command` VALUES
('ticket',2,'Syntax: .ticket #id\r\n\r\nActs as an alias of: ".ticket read"\r\n'),
('ticket whisper',2,'Syntax: .ticket whisper #id $message\r\n\r\nAttempt to answer in-game GM ticket with number #id by sending whisper with $message. Ticket will be assigned regardless of author\'s online status.'),
('tickets',2,'Syntax: .tickets [on|off|[#categoryid #max|#max] [online]]\r\n\r\nIf "on"/"off" provided, enable or disable in-game GM ticket queue notifications and GM ticket alerts. Acts as an alias of ".tickets list" otherwise.\r\n'),
('tickets list',2,'Syntax: .tickets list [#categoryid #max|#max] [online]\r\n\r\nShow current GM tickets queue. If #categoryid is provided, show only GM tickets from that category.');
/*!40000 ALTER TABLE `command` ENABLE KEYS */;
UNLOCK TABLES;

LOCK TABLES `mangos_string` WRITE;
/*!40000 ALTER TABLE `mangos_string` DISABLE KEYS */;
DELETE FROM `mangos_string` WHERE `entry` IN (1524, 1530, 1548, 1574) OR (`entry` > 1577 AND `entry` < 1591);
INSERT INTO `mangos_string` VALUES
(1524,'| Quick actions: |c00FFFFFF<Shift+click>|r on ticket id tag to read the ticket',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1530,'|========================================================|',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1548,'[Tickets]: [%s] has been successfully surveyed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1574,'| Quick actions: %s %s %s %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1578,'SORT',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1579,'NOTE',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1580,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1581,'Ticket notifications',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1582,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1583,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1584,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1585,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1586,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1587,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1588,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1589,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1590,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `mangos_string` ENABLE KEYS */;
UNLOCK TABLES;
