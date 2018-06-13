ALTER TABLE realmd_db_version CHANGE COLUMN required_z2716_01_realmd_totp required_z2720_01_realmd_warden_system bit;

ALTER TABLE account ADD COLUMN os VARCHAR(3) DEFAULT '' AFTER locale;

DROP TABLE IF EXISTS `warden_log`;
CREATE TABLE `warden_log` (
  `entry` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Log entry ID',
  `check` smallint(5) unsigned NOT NULL COMMENT 'Failed Warden check ID',
  `action` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Action taken (enum WardenActions)',
  `account` int(11) unsigned NOT NULL COMMENT 'Account ID',
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Player GUID',
  `map` int(11) unsigned DEFAULT NULL COMMENT 'Map ID',
  `position_x` float DEFAULT NULL COMMENT 'Player position X',
  `position_y` float DEFAULT NULL COMMENT 'Player position Y',
  `position_z` float DEFAULT NULL COMMENT 'Player position Z',
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Date of the log entry',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Warden log of failed checks';
