ALTER TABLE db_version CHANGE COLUMN required_z2725_01_mangos_trainer_greeting required_z2726_01_mangos_creature_conditional_spawn bit;

--
-- Table structure for table `creature_conditional_spawn`
--

DROP TABLE IF EXISTS `creature_conditional_spawn`;
CREATE TABLE `creature_conditional_spawn` (
  `Guid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Global Unique Identifier',
  `EntryAlliance` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Alliance Creature Identifier',
  `EntryHorde` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Horde Creature Identifier',
  `Comments` varchar(255) NOT NULL,
  PRIMARY KEY (`Guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Conditional Spawn)';

--
-- Dumping data for table `creature_conditional_spawn`
--

LOCK TABLES `creature_conditional_spawn` WRITE;
/*!40000 ALTER TABLE `creature_conditional_spawn` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_conditional_spawn` ENABLE KEYS */;
UNLOCK TABLES;
