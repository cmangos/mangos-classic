DROP TABLE IF EXISTS `playerbot_saved_data`;
CREATE TABLE `playerbot_saved_data` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `combat_order` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `primary_target` int(11) unsigned NOT NULL DEFAULT '0',
  `secondary_target` int(11) unsigned NOT NULL DEFAULT '0',
  `pname` varchar(12) NOT NULL DEFAULT '',
  `sname` varchar(12) NOT NULL DEFAULT '',
  `combat_delay` INT(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;