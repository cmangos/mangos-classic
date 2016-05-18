CREATE TABLE `cheaters` (
 `entry` bigint(20) NOT NULL auto_increment,
  `player` varchar(30) NOT NULL,
  `acctid` int(11) NOT NULL,
  `reason` varchar(255) NOT NULL default 'unknown',
  `speed` float NOT NULL default '0',
  `Val1` float NOT NULL default '0',
  `Val2` int(10) unsigned NOT NULL default '0',
  `count` int(11) NOT NULL default '0',
  `Map` smallint(5) NOT NULL default '-1',
  `Pos` varchar(255) NOT NULL default '0',
  `Level` mediumint(9) NOT NULL default '0',
  `first_date` datetime NOT NULL,
  `last_date` datetime NOT NULL,
  `Op` varchar(255) NOT NULL default 'unknown',
  PRIMARY KEY  (`entry`),
  KEY `idx_Count` (`count`),
  KEY `idx_Player` (`player`)
) ENGINE=InnoDB AUTO_INCREMENT=30 DEFAULT CHARSET=utf8;