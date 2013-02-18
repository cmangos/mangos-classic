ALTER TABLE db_version CHANGE COLUMN required_z2241_s1878_12344_01_mangos_command required_z2245_s1882_12349_01_mangos_dbscripts_on_creature_death bit;


--
-- Table structure for table `dbscripts_on_creature_death`
--

DROP TABLE IF EXISTS `dbscripts_on_creature_death`;
CREATE TABLE `dbscripts_on_creature_death` (
  `id` mediumint(8) unsigned NOT NULL default '0',
  `delay` int(10) unsigned NOT NULL default '0',
  `command` mediumint(8) unsigned NOT NULL default '0',
  `datalong` mediumint(8) unsigned NOT NULL default '0',
  `datalong2` int(10) unsigned NOT NULL default '0',
  `buddy_entry` int(10) unsigned NOT NULL default '0',
  `search_radius` int(10) unsigned NOT NULL default '0',
  `data_flags` tinyint(3) unsigned NOT NULL default '0',
  `dataint` int(11) NOT NULL default '0',
  `dataint2` int(11) NOT NULL default '0',
  `dataint3` int(11) NOT NULL default '0',
  `dataint4` int(11) NOT NULL default '0',
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `z` float NOT NULL default '0',
  `o` float NOT NULL default '0',
  `comments` varchar(255) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
