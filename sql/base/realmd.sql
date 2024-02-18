-- MySQL dump 10.13
--
-- Host: localhost    Database: realmd
-- ------------------------------------------------------
-- Server version	5.5.32

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `realmd_db_version`
--

DROP TABLE IF EXISTS `realmd_db_version`;
CREATE TABLE `realmd_db_version` (
  `required_z2820_01_realmd_joindate_datetime` bit(1) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Last applied sql update to DB';

--
-- Dumping data for table `realmd_db_version`
--

LOCK TABLES `realmd_db_version` WRITE;
/*!40000 ALTER TABLE `realmd_db_version` DISABLE KEYS */;
INSERT INTO `realmd_db_version` VALUES
(NULL);
/*!40000 ALTER TABLE `realmd_db_version` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `username` varchar(32) NOT NULL DEFAULT '',
  `gmlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `sessionkey` longtext,
  `v` longtext,
  `s` longtext,
  `email` text,
  `joindate` DATETIME NOT NULL DEFAULT NOW(),
  `lockedIp` varchar(30) NOT NULL DEFAULT '0.0.0.0',
  `failed_logins` int(11) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `last_module` char(32) DEFAULT '',
  `module_day` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `active_realm_id` int(11) unsigned NOT NULL DEFAULT '0',
  `expansion` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mutetime` bigint(40) unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) NOT NULL DEFAULT '',
  `os` VARCHAR(4) NOT NULL DEFAULT '0',
  `platform` VARCHAR(4) NOT NULL DEFAULT '0',
  `token` text,
  `flags` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_username` (`username`),
  KEY `idx_gmlevel` (`gmlevel`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Account System';

--
-- Dumping data for table `account`
--

LOCK TABLES `account` WRITE;
/*!40000 ALTER TABLE `account` DISABLE KEYS */;
INSERT INTO `account` VALUES
(1,'ADMINISTRATOR',3,'','312B99EEF1C0196BB73B79D114CE161C5D089319E6EF54FAA6117DAB8B672C14','8EB5DE915AA3D805FA7099CF61C0BB8A77990EA869078A0C5B9EEE55828F4505','','2006-04-25 10:18:56','127.0.0.1',0,0,'',0,0,0,0,'',0,0,NULL,0),
(2,'GAMEMASTER',2,'','681F5A7D4DE26DBFD3060EE37E03B79FD154875FB18F44DBF843963F193FC1AC','8873CD861DEFBF124232D6A29E4884E34C73385304A8AC44175976B1003DCFD7','','2006-04-25 10:18:56','127.0.0.1',0,0,'',0,0,0,0,'',0,0,NULL,0),
(3,'MODERATOR',1,'','2CA85C9853E44A6DCE09FC92EBDE57EF20975281EB7604326E25751AF8576859','AD68B088D7BCE5E4B734495A7A956F1D5DD1BAB61FB0FEE46C737D93EC166DF5','','2006-04-25 10:19:35','127.0.0.1',0,0,'',0,0,0,0,'',0,0,NULL,0),
(4,'PLAYER',0,'','3738EC7E7C731FD431C716990C6D97CA5C1D50EF0DA7DE9819076DE1D03AA891','EBA23AF194D89B8061CA7FEBA06D336B1C38D8FBDABA76F2C51D45141362D881','','2006-04-25 10:19:35','127.0.0.1',0,0,'',0,0,0,0,'',0,0,NULL,0);
/*!40000 ALTER TABLE `account` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `account_banned`
--

DROP TABLE IF EXISTS `account_banned`;
CREATE TABLE `account_banned` (
  `id` int(11) AUTO_INCREMENT,
  `account_id` int(11) NOT NULL DEFAULT '0' COMMENT 'Account id',
  `banned_at` bigint(40) NOT NULL DEFAULT '0',
  `expires_at` bigint(40) NOT NULL DEFAULT '0',
  `banned_by` varchar(50) NOT NULL,
  `unbanned_at` bigint(40) NOT NULL DEFAULT '0',
  `unbanned_by` varchar(50) DEFAULT NULL,
  `reason` varchar(255) NOT NULL,
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Ban List';

--
-- Dumping data for table `account_banned`
--

LOCK TABLES `account_banned` WRITE;
/*!40000 ALTER TABLE `account_banned` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_banned` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS `account_logons`;
CREATE TABLE `account_logons` (
`id` INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
`accountId` INT UNSIGNED NOT NULL,
`ip` varchar(30) NOT NULL,
`loginTime` timestamp NOT NULL,
`loginSource` INT UNSIGNED NOT NULL
);

DROP TABLE IF EXISTS account_raf;
CREATE TABLE account_raf(
referrer INT UNSIGNED NOT NULL DEFAULT '0',
referred INT UNSIGNED NOT NULL DEFAULT '0',
PRIMARY KEY(referrer, referred)
);

--
-- Table structure for table `ip_banned`
--

DROP TABLE IF EXISTS `ip_banned`;
CREATE TABLE `ip_banned` (
  `ip` varchar(32) NOT NULL DEFAULT '0.0.0.0',
  `banned_at` bigint(40) NOT NULL,
  `expires_at` bigint(40) NOT NULL,
  `banned_by` varchar(50) NOT NULL DEFAULT '[Console]',
  `reason` varchar(255) NOT NULL DEFAULT 'no reason',
  PRIMARY KEY (`ip`,`banned_at`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Banned IPs';

--
-- Dumping data for table `ip_banned`
--

LOCK TABLES `ip_banned` WRITE;
/*!40000 ALTER TABLE `ip_banned` DISABLE KEYS */;
/*!40000 ALTER TABLE `ip_banned` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `realmcharacters`
--

DROP TABLE IF EXISTS `realmcharacters`;
CREATE TABLE `realmcharacters` (
  `realmid` int(11) unsigned NOT NULL DEFAULT '0',
  `acctid` bigint(20) unsigned NOT NULL,
  `numchars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`acctid`),
  KEY `acctid` (`acctid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm Character Tracker';

--
-- Dumping data for table `realmcharacters`
--

LOCK TABLES `realmcharacters` WRITE;
/*!40000 ALTER TABLE `realmcharacters` DISABLE KEYS */;
/*!40000 ALTER TABLE `realmcharacters` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `realmlist`
--

DROP TABLE IF EXISTS `realmlist`;
CREATE TABLE `realmlist` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL DEFAULT '',
  `address` varchar(32) NOT NULL DEFAULT '127.0.0.1',
  `port` int(11) NOT NULL DEFAULT '8085',
  `icon` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `realmflags` tinyint(3) unsigned NOT NULL DEFAULT '2' COMMENT 'Supported masks: 0x1 (invalid, not show in realm list), 0x2 (offline, set by mangosd), 0x4 (show version and build), 0x20 (new players), 0x40 (recommended)',
  `timezone` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `allowedSecurityLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `population` float unsigned NOT NULL DEFAULT '0',
  `realmbuilds` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm System';

--
-- Dumping data for table `realmlist`
--

LOCK TABLES `realmlist` WRITE;
/*!40000 ALTER TABLE `realmlist` DISABLE KEYS */;
INSERT INTO `realmlist` VALUES
(1,'MaNGOS','127.0.0.1',8085,1,0,1,0,0,'');
/*!40000 ALTER TABLE `realmlist` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `uptime`
--

DROP TABLE IF EXISTS `uptime`;
CREATE TABLE `uptime` (
  `realmid` int(11) unsigned NOT NULL,
  `starttime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startstring` varchar(64) NOT NULL DEFAULT '',
  `uptime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `maxplayers` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`starttime`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Uptime system';

--
-- Dumping data for table `uptime`
--

LOCK TABLES `uptime` WRITE;
/*!40000 ALTER TABLE `uptime` DISABLE KEYS */;
/*!40000 ALTER TABLE `uptime` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

DROP TABLE IF EXISTS `antispam_detected`;
DROP TABLE IF EXISTS `antispam_mute`;
DROP TABLE IF EXISTS `antispam_scores`;

DROP TABLE IF EXISTS `system_fingerprint_usage`;

CREATE TABLE `system_fingerprint_usage` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `fingerprint` int(10) unsigned NOT NULL,
  `account` int(10) unsigned NOT NULL,
  `ip` varchar(16) NOT NULL,
  `realm` int(10) unsigned NOT NULL,
  `time` datetime DEFAULT NULL,
  `architecture` varchar(16) DEFAULT NULL,
  `cputype` varchar(64) DEFAULT NULL,
  `activecpus` int(10) unsigned DEFAULT NULL,
  `totalcpus` int(10) unsigned DEFAULT NULL,
  `pagesize` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fingerprint` (`fingerprint`),
  KEY `account` (`account`),
  KEY `ip` (`ip`)
) ENGINE=InnoDB AUTO_INCREMENT=77 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `antispam_replacement`;

CREATE TABLE `antispam_replacement` (
  `from` varchar(32) NOT NULL DEFAULT '',
  `to` varchar(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`from`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

insert  into `antispam_replacement`(`from`,`to`) values ('\\\\/\\\\/','W'),('/\\/\\','M'),('0','O'),('...hic!',''),('()','O'),('\\/\\/','W'),('/\\\\','A'),('VV','W'),('@','O'),('/V\\','M'),('/\\\\/\\\\','M'),('㎜','MM'),('!<','K');

DROP TABLE IF EXISTS `antispam_unicode`;
DROP TABLE IF EXISTS `antispam_unicode_replacement`;

CREATE TABLE `antispam_unicode_replacement` (
  `from` mediumint(5) unsigned NOT NULL DEFAULT '0',
  `to` mediumint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`from`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

insert  into `antispam_unicode_replacement`(`from`,`to`) values (1063,52),(1054,79),(1057,67),(1052,77),(927,79),(1050,75),(913,65),(917,69),(1062,85),(9675,79),(1040,65),(1058,84),(1064,87),(1025,69),(1055,78),(1065,87),(922,75),(924,77),(1045,69),(968,87),(192,65),(210,79),(211,79),(242,79),(324,78),(328,78),(332,79),(466,79),(59336,78),(12562,84),(8745,78),(65325,77),(959,79),(945,65),(954,75),(12295,79),(65323,75),(65296,79),(65355,75),(65357,77),(65319,71),(925,78);

DROP TABLE IF EXISTS `antispam_blacklist`;

CREATE TABLE `antispam_blacklist` (
  `string` varchar(64) NOT NULL,
  PRIMARY KEY (`string`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `antispam_blacklist` */

insert  into `antispam_blacklist`(`string`) values 
(' <<<10G=1$--and'),
(' http://400elysium.tk/ '),
(' W W W .N O S T 100.C O M !50 %'),
(' www.aoaue.com'),
(' [www.y2IgoId.com>10g=2$/Power] '),
('\"COM\"'),
('\"T100\"'),
('\"T100\" plus \"COM\"<<'),
(',finally we got 500g\r\n,finally w'),
('.c¡£ 0m'),
('100COM'),
('10g='),
('1OOCOM'),
('2$/Power]'),
('300elysium.tk'),
('300ELYSIUMTK'),
('400elysium.tk/'),
('400ELYSIUMTK'),
('4GAMEPOWER'),
('4WOWCOM'),
('50G=10.00$'),
('7 days non-stop hard working,fin'),
('<<<100G=12$--and'),
('<<<10G=1$'),
('>>\"NOS\"'),
('?\\/!CT0RYW0WC0M'),
('aoaue'),
('aoauecom'),
('com>10g'),
('elysium-gifts.tk'),
('elysium-projecttk'),
('ELYSIUMABCWOW'),
('ELYSIUMAEWOW'),
('ELYSIUMAPKWOW'),
('ELYSIUMDFTWOW'),
('ELYSIUMFDCWOW'),
('ELYSIUMGIFTSTK'),
('ELYSIUMKSDWOW'),
('ELYSIUMKYWOW'),
('ELYSIUMLOLWOW'),
('ELYSIUMORWOW'),
('ELYSIUMPROJECTTK'),
('ELYSIUMRDWOW'),
('ELYSIUMRSWOW'),
('ELYSIUMSAYWOW'),
('ELYSIUMSHEWOW'),
('ELYSIUMSNKWOW'),
('ELYSIUMTXTWOW'),
('ELYSIUMUFNWOW'),
('ELYSIUMUSAWOW'),
('ELYSIUMWOWTK'),
('ELYSIUMXGHWOW'),
('ELYSIUMXXXWOW'),
('ELYSIUMZZZWOW'),
('EPICITEMSHONOR'),
('FASTLVLCOM'),
('finally we got 500g'),
('finallywegot500g'),
('Flask/Epic items/Honor'),
('g(2+2)wowcom'),
('G-4-VV-0-W--C--0--M'),
('G4W@W'),
('G4WOW'),
('G4WOWCOM'),
('GFOURWOWCOM'),
('GO1D4MMO'),
('GOALDMOCOM'),
('GoId/eppic items/Bags'),
('GOLADAMOCOM'),
('GOLADMOCOM'),
('GOLD4MMO'),
('GOLDAMOCOM'),
('GOLDCEO'),
('GOLDCOM'),
('GOLDDEALRU'),
('GOLDINSIDER'),
('GOLDPOWER'),
('GOLODMOCOM'),
('GOLODOMOCOM'),
('GOXLD4MOCOM'),
('GOXLD4XMMOCOM'),
('GOXLDX4MXMOCOM'),
('GOXLDXMOCOM'),
('Greetingsafter7days'),
('http://elysium-pro.tk'),
('http://elysium-project.tk'),
('HTTPSELySIUMProJECtTK'),
('HTTPSWWWELYSIUMGIFTSTK'),
('HTTPSWWWELYSIUMPROJECTTK'),
('HTTPSWWWY2IGOIDCOM>'),
('HTTPWWWY2IGOIDCOM>'),
('ILOVEUGOLD'),
('ITEM4GAME'),
('ITEM4WOW'),
('ITEM4WOW.COM'),
('ITEM4WOW.COM<---here'),
('ITEM4WOWCOM'),
('LAODAN8841'),
('leveling1-60=350$'),
('LOD.COM<<<'),
('LODCOM<<<---)('),
('LOVEWOWHAHA'),
('LOVEWOWHAHACOM'),
('love£¨wow£©haha'),
('LV60/ ---'),
('LVLGOCOM'),
('MMO4PAL'),
('MMOANKCOM'),
('MMOGOCOM'),
('MMOGSCOM'),
('MMOLORD'),
('MMOOKCOM'),
('MMOSECOM'),
('MMOTANKCOM'),
('MONEYCIRCLERU'),
('MONEYFORGAMES'),
('NAXXGAMES'),
('NAXXGAMESCOM'),
('NIGHTHAVEN20G'),
('NOST!OO'),
('NOST100'),
('NOST1OO'),
('OKOGAME'),
('OKOGAMES'),
('OKOGOMES'),
('owhaha.com'),
('OWHAHACOM'),
('power1-60'),
('POWERLV60'),
('PVPBANK'),
('PVPBCOMANK'),
('QQ1353341694'),
('QQ211772670'),
('QQ373353356'),
('RNRNOOK'),
('RNRNOSE'),
('SAGEBLADECOM'),
('SELLNGOLD'),
('SINBAGAME'),
('SINBAGOLD'),
('SINBAONLINE'),
('SKYPEBAUDIEA'),
('SKYPEBRAT7TH'),
('SKYPEELYSUIM'),
('SKYPEELYSUIMKFWOW'),
('SKYPEELYSUIMWOWOW'),
('SKYPEEMIL8LI'),
('SKYPEFELWOOD'),
('SKYPEJASZUINS'),
('SKYPEJAZZYTSWOW'),
('SKYPELAR2IDA'),
('SKYPEMOONCLOTH'),
('SKYPEPROJECTERWOW'),
('SKYPESPRAGUEI'),
('SKYPEVANILLA'),
('Stockof60characters'),
('SUSANGAME'),
('T100'),
('T100plusCOM'),
('two weeks of non-stop hard worki'),
('URL divide into three'),
('URLintothreepart'),
('V1CTORYWOW'),
('VICTORYW0W'),
('VICTORYWOW'),
('VICTORYWOW.COM'),
('VICTORYWOWCOM'),
('working,finally we got 500g'),
('WOWJEVERLY'),
('WOWMARY'),
('WTSITEM'),
('www.aoaue.com'),
('WWW.G(2+2)WOW.COM'),
('WWW.G-4-VV-0-W--C--0--M'),
('WWW.G4WOW.COM'),
('www.lovewowhah'),
('www.love£¨wow£©haha .com'),
('www.love£¨wow£©haha .com________'),
('www.love£¨wow£©haha .c¡£ 0m'),
('www.y2IgoId.com'),
('www.y2IgoId.com>'),
('www.y2IgoId.com>10g=2$/Power'),
('www.y2IgoId.com>10g=2$/Power LV6'),
('WWW.Y2LGOLD.COM'),
('www.y2lgold.com>'),
('WWW.Y2LGOLD.COM>Gold'),
('WWWELYSIUMGIFTSTK'),
('WWWELYSIUMPROJECTTK'),
('WWWELYSIUMWOWTK'),
('WWWG4WOWCOM'),
('WWWMMOTANK'),
('WWWNAXXGAMESCOM'),
('WWWNOST'),
('WWWNOST100COM'),
('wwwy2IgoIdcom'),
('WWWY2IGOIDCOM>'),
('WWWY2LGOLD.COM'),
('WWWY2LGOLDCOM'),
('WWWY@LGOLDCOM'),
('X2GOLD'),
('XIAOBAIMOSHOUJINGJI'),
('y(1+1)lgold'),
('y2IgoId'),
('y2IgoId.com'),
('y2IgoIdcom'),
('Y2LGOLD'),
('Y2LGOLD.COM'),
('Y2LGOLDCOM'),
('YOURGNET'),
('[GM]'),
('[www.y2IgoId.com>10g=2$/Power] L');


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-09-10  0:00:00
