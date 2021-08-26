ALTER TABLE db_version CHANGE COLUMN required_z2777_01_mangos_corpse_decay required_z2778_01_mangos_anticheat bit;

/*
SQLyog Community v13.1.5  (64 bit)
MySQL - 5.7.20-log : Database - vengeance_world
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `warden_scans` */

DROP TABLE IF EXISTS `warden_scans`;

CREATE TABLE `warden_scans` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `type` int(2) DEFAULT '0',
  `str` text,
  `data` text,
  `address` int(8) DEFAULT '0',
  `length` int(2) DEFAULT '0',
  `result` tinytext NOT NULL,
  `flags` smallint(5) unsigned NOT NULL,
  `comment` tinytext NOT NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=83 DEFAULT CHARSET=utf8;

/*Data for the table `warden_scans` */

insert  into `warden_scans`(`id`,`type`,`str`,`data`,`address`,`length`,`result`,`flags`,`comment`) values 
(1,0,NULL,NULL,8679268,6,'686561646572',6,'Packet internal sign - \"header\"'),
(2,0,NULL,NULL,8696620,6,'686561646572',8,'Packet internal sign - \"header\"'),
(3,0,NULL,NULL,8530960,6,'53595354454D',6,'Packet internal sign - \"SYSTEM\"'),
(4,0,NULL,NULL,8547832,6,'53595354454D',8,'Packet internal sign - \"SYSTEM\"'),
(5,2,NULL,'82D7E5CBC8D2F78A791E189BAB3FD5D4342BF7EB0CA3F129',74044,NULL,'0',14,'Cheat Engine dll'),
(6,2,NULL,'A444519CC419521B6D39990C1D95329C8D94B59226CBAA98',16507,NULL,'0',14,'WPE PRO dll'),
(7,2,NULL,'3A0F8985E701343E439C74B675C72BBE2D8810A745569913',372624,NULL,'0',14,'rPE dll'),
(8,0,NULL,NULL,8151666,4,'D893FEC0',2,'Jump gravity'),
(9,0,NULL,NULL,8151646,2,'3075',2,'Jump gravity water'),
(10,0,NULL,NULL,6382555,2,'8A47',2,'Anti root'),
(11,0,NULL,NULL,6380789,1,'F8',2,'Anti move'),
(12,0,NULL,NULL,8151647,1,'75',2,'Anti jump'),
(13,0,NULL,NULL,8152026,4,'8B4F7889',2,'No fall damage'),
(14,0,NULL,NULL,6504892,2,'7425',2,'Super fly'),
(15,0,NULL,NULL,6383433,2,'780F',2,'Heartbeat interval speedhack'),
(16,0,NULL,NULL,6284623,1,'F4',2,'Anti slow hack'),
(17,0,NULL,NULL,6504931,2,'85D2',2,'No fall damage'),
(18,0,NULL,NULL,8151565,2,'2000',2,'Fly hack'),
(19,0,NULL,NULL,7153475,6,'890D509CCE00',2,'General hacks'),
(20,0,NULL,NULL,7138894,6,'A3D89BCE00EB',2,'Wall climb'),
(21,0,NULL,NULL,7138907,6,'890DD89BCE00',2,'Wall climb'),
(22,0,NULL,NULL,6993044,1,'74',2,'Zero gravity'),
(23,0,NULL,NULL,6502300,1,'FC',2,'Air walk'),
(24,0,NULL,NULL,6340512,2,'7F7D',2,'Wall climb'),
(25,0,NULL,NULL,6380455,4,'F4010000',2,'Wall climb'),
(26,0,NULL,NULL,8151657,4,'488C11C1',2,'Wall climb'),
(27,0,NULL,NULL,6992319,3,'894704',2,'Wall climb'),
(28,0,NULL,NULL,6340529,2,'746C',2,'No water hack'),
(29,0,NULL,NULL,6356016,10,'C70588D8C4000C000000',2,'No water hack'),
(30,0,NULL,NULL,4730584,6,'0F8CE1000000',2,'WMO collision'),
(31,0,NULL,NULL,4803152,7,'A1C0EACE0085C0',2,'noclip hack'),
(32,0,NULL,NULL,5946704,6,'8BD18B0D80E0',2,'M2 collision'),
(33,0,NULL,NULL,6340543,2,'7546',2,'M2 collision'),
(34,0,NULL,NULL,5341282,1,'7F',2,'Warden disable'),
(35,0,NULL,NULL,4989376,1,'72',2,'No fog hack'),
(36,0,NULL,NULL,8145237,1,'8B',2,'No fog hack'),
(37,0,NULL,NULL,6392083,8,'8B450850E824DA1A',2,'No fog hack'),
(38,0,NULL,NULL,8146241,10,'D9818C0000008BE55DC2',2,'tp2plane hack'),
(39,0,NULL,NULL,6995731,1,'74',2,'Air swim hack'),
(40,0,NULL,NULL,6964859,1,'75',2,'Infinite jump hack'),
(41,0,NULL,NULL,6382558,10,'84C074178B86A4000000',2,'Gravity water hack'),
(42,0,NULL,NULL,8151997,3,'895108',2,'Gravity hack'),
(43,0,NULL,NULL,8152025,1,'34',2,'Plane teleport'),
(44,0,NULL,NULL,6516436,1,'FC',2,'Zero fall time'),
(45,0,NULL,NULL,6501616,1,'FC',2,'No fall damage'),
(46,0,NULL,NULL,6511674,1,'FC',2,'Fall time hack'),
(47,0,NULL,NULL,6513048,1,'FC',2,'Death bug hack'),
(48,0,NULL,NULL,6514072,1,'FC',2,'Anti slow hack'),
(49,0,NULL,NULL,8152029,3,'894E38',2,'Anti slow hack'),
(50,0,NULL,NULL,4847346,3,'8B45D4',2,'Max camera distance hack'),
(51,0,NULL,NULL,4847069,1,'74',2,'Wall climb'),
(52,0,NULL,NULL,8155231,3,'000000',2,'Signature check'),
(53,0,NULL,NULL,6356849,1,'74',2,'Signature check'),
(54,0,NULL,NULL,6354889,6,'0F8A71FFFFFF',2,'Signature check'),
(55,0,NULL,NULL,4657642,1,'74',2,'Max interact distance hack'),
(56,0,NULL,NULL,6211360,8,'558BEC83EC0C8B45',2,'Hover speed hack'),
(57,0,NULL,NULL,8153504,3,'558BEC',2,'Flight speed hack'),
(58,0,NULL,NULL,6214285,6,'8B82500E0000',2,'Track all units hack'),
(59,0,NULL,NULL,8151558,11,'25FFFFDFFB0D0020000089',2,'No fall damage'),
(60,0,NULL,NULL,8155228,6,'89868C000000',2,'Run speed hack'),
(61,0,NULL,NULL,6356837,2,'7474',2,'Follow anything hack'),
(62,0,NULL,NULL,6751806,1,'74',2,'No water hack'),
(63,0,NULL,NULL,4657632,2,'740A',2,'Any name hack'),
(64,0,NULL,NULL,8151976,4,'84E5FFFF',2,'Plane teleport'),
(65,0,NULL,NULL,6214371,6,'8BB1540E0000',2,'Object tracking hack'),
(66,0,NULL,NULL,6818689,5,'A388F2C700',2,'No water hack'),
(67,0,NULL,NULL,6186028,5,'C705ACD2C4',2,'No fog hack'),
(68,0,NULL,NULL,5473808,4,'30855300',2,'Warden disable hack '),
(69,0,NULL,NULL,4208171,3,'6B2C00',2,'Warden disable hack'),
(70,0,NULL,NULL,7119285,1,'74',2,'Warden disable hack'),
(71,0,NULL,NULL,4729827,1,'5E',2,'Daylight hack'),
(72,0,NULL,NULL,6354512,6,'0F84EA000000',2,'Ranged attack stop hack'),
(73,0,NULL,NULL,5053463,2,'7415',2,'Officer note hack'),
(74,4,'World\\Lordaeron\\stratholme\\Activedoodads\\doors\\nox_door_plague.m2',NULL,0,0,'B4452B6D95C98B186A70B008FA07BBAEF30DF7A2',14,'Stratholme door'),
(75,4,'World\\Kalimdor\\onyxiaslair\\doors\\OnyxiasGate01.m2',NULL,0,0,'75195E4AEDA0BCAF048CA0E34D95A70D4F53C746',14,'Onyxia gate'),
(76,4,'World\\Generic\\Human\\Activedoodads\\doors\\deadminedoor02.m2',NULL,0,0,'3DFF011B9AB134F37F885097E695351B91953564',14,'Deadmines door'),
(77,4,'World\\Kalimdor\\silithus\\activedoodads\\ahnqirajdoor\\ahnqirajdoor02.m2',NULL,0,0,'DBD4F407C468CC36134E621D160178FDA4D0D249',14,'AQ door'),
(78,4,'World\\Kalimdor\\diremaul\\activedoodads\\doors\\diremaulsmallinstancedoor.m2',NULL,0,0,'0DC8DB46C85549C0FF1A600F6C236357C305781A',14,'Dire Maul Gordok Inner Door'),
(79,0,NULL,NULL,8139737,5,'D84E14DEC1',2,'UNKNOWN movement hack?'),
(80,0,NULL,NULL,8902804,4,'8E977042',2,'Wall climb hack'),
(81,0,NULL,NULL,8902808,4,'0000E040',2,'Run speed hack'),
(82,0,NULL,NULL,8154755,7,'8166403FFFDFFF',2,'Moveflag hack'),
(83,0,NULL,NULL,8445948,4,'BB8D243F',2,'Wall climb hack'),
(84,0,NULL,NULL,6493717,2,'741D',2,'Speed hack'),
(85,2,NULL,'33D233C9E887071B00E8',13856,NULL,'1',14,'Warden packet process code search sanity check'),
(86,1,'kernel32.dll',NULL,0,0,'1',14,'Warden module search bypass sanity check'),
(87,1,'wpespy.dll',NULL,0,0,'0',14,'WPE Pro'),
(88,1,'speedhack-i386.dll',NULL,0,0,'0',14,'CheatEngine'),
(89,1,'tamia.dll',NULL,0,0,'0',14,'Tamia hack');


DROP TABLE IF EXISTS `warden_check_driver`; 
DROP TABLE IF EXISTS `warden_check_lua`; 
DROP TABLE IF EXISTS `warden_check_memory`; 
DROP TABLE IF EXISTS `warden_check_memory_dynamic`; 
DROP TABLE IF EXISTS `warden_check_module`; 
DROP TABLE IF EXISTS `warden_check_mpq`; 
DROP TABLE IF EXISTS `warden_check_page_a`; 
DROP TABLE IF EXISTS `warden_check_page_b`; 
DROP TABLE IF EXISTS `warden_module`;

REPLACE INTO mangos_string VALUES
(67,'|c00FFFFFF|Announce:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
