ALTER TABLE character_db_version CHANGE COLUMN required_z2745_01_characters_weekly_quests required_z2751_01_characters_new_ticket_system bit;

--
-- Table structure for table `gm_tickets`
--

DROP TABLE IF EXISTS `gm_tickets`;
CREATE TABLE `gm_tickets` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'GM Ticket\'s unique identifier',
  `category` tinyint(2) unsigned NOT NULL DEFAULT '0' COMMENT 'GM Ticket Category DBC entry\'s identifier',
  `state` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'Ticket\'s current state',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'Ticket\'s current status',
  `level` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'Ticket\'s security level',
  `author_guid` int(11) unsigned NOT NULL COMMENT 'Player\'s character Global Unique Identifier',
  `author_name` varchar(12) NOT NULL COMMENT 'Player\'s character name',
  `locale` varchar(4) NOT NULL DEFAULT 'enUS' COMMENT 'Player\'s client locale name',
  `mapid` int(4) unsigned NOT NULL DEFAULT '0' COMMENT 'Character\'s map identifier on submission',
  `x` float NOT NULL DEFAULT '0' COMMENT 'Character\'s x coordinate on submission',
  `y` float NOT NULL DEFAULT '0' COMMENT 'Character\'s y coordinate on submission',
  `z` float NOT NULL DEFAULT '0' COMMENT 'Character\'s z coordinate on submission',
  `o` float NOT NULL DEFAULT '0' COMMENT 'Character\'s orientation angle on submission',
  `text` text NOT NULL COMMENT 'Ticket\'s message',
  `created` bigint(40) unsigned NOT NULL COMMENT 'Timestamp: ticket created by a player',
  `updated` bigint(40) unsigned NOT NULL DEFAULT 0 COMMENT 'Timestamp: ticket text\'s last update',
  `seen` bigint(40) unsigned NOT NULL DEFAULT 0 COMMENT 'Timestamp: ticket\'s last time opened by a GM',
  `answered` bigint(40) unsigned NOT NULL DEFAULT 0 COMMENT 'Timestamp: ticket\'s last time answered by a GM',
  `closed` bigint(40) unsigned NOT NULL DEFAULT 0 COMMENT 'Timestamp: ticket closed by a GM',
  `assignee_guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Assignee\'s character\'s Global Unique Identifier',
  `assignee_name` varchar(12) NOT NULL DEFAULT '' COMMENT 'Assignee\'s character\'s name',
  `conclusion` varchar(255) NOT NULL DEFAULT '' COMMENT 'Assignee\'s final conclusion on this ticket',
  `notes` varchar(10000) NOT NULL DEFAULT '' COMMENT 'Additional notes for GMs',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='GM Tickets System';

--
-- Dumping data for table `gm_tickets`
--

LOCK TABLES `gm_tickets` WRITE;
/*!40000 ALTER TABLE `gm_tickets` DISABLE KEYS */;
/*!40000 ALTER TABLE `gm_tickets` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gm_surveys`
--

DROP TABLE IF EXISTS `gm_surveys`;
CREATE TABLE `gm_surveys` (
  `ticketid` int(11) unsigned NOT NULL COMMENT 'GM Ticket\'s unique identifier',
  `surveyid` int(2) unsigned NOT NULL COMMENT 'Survey DBC entry\'s identifier',
  `answer1` tinyint(2) unsigned,
  `answer2` tinyint(2) unsigned,
  `answer3` tinyint(2) unsigned,
  `answer4` tinyint(2) unsigned,
  `answer5` tinyint(2) unsigned,
  `answer6` tinyint(2) unsigned,
  `answer7` tinyint(2) unsigned,
  `answer8` tinyint(2) unsigned,
  `answer9` tinyint(2) unsigned,
  `answer10` tinyint(2) unsigned,
  `comment` text COMMENT 'Player\'s feedback',
  PRIMARY KEY (`ticketid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='GM Tickets System';

--
-- Dumping data for table `gm_surveys`
--

LOCK TABLES `gm_surveys` WRITE;
/*!40000 ALTER TABLE `gm_surveys` DISABLE KEYS */;
/*!40000 ALTER TABLE `gm_surveys` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Migrating data from table `character_tickets` to table `gm_tickets`
--

LOCK TABLES `gm_tickets` WRITE, `characters` AS C WRITE, `character_ticket` AS T WRITE;
/*!40000 ALTER TABLE `gm_tickets` DISABLE KEYS */;
INSERT INTO `gm_tickets` (`author_guid`, `author_name`, `text`, `created`) SELECT T.`guid`, C.`name`, T.`ticket_text`, UNIX_TIMESTAMP() FROM `character_ticket` T INNER JOIN `characters` C ON T.`guid` = C.`guid` ORDER BY T.`ticket_id`;
/*!40000 ALTER TABLE `gm_tickets` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS `character_ticket`;

