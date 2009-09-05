ALTER TABLE character_db_version CHANGE COLUMN required_066_8402_02_characters_guild_bank_eventlog required_066_8402_01_characters_guild_eventlog bit;

DROP TABLE IF EXISTS `guild_bank_eventlog`;
CREATE TABLE `guild_bank_eventlog` (
  `guildid` int(11) unsigned NOT NULL default '0',
  `LogGuid` int(11) unsigned NOT NULL default '0',
  `LogEntry` tinyint(1) unsigned NOT NULL default '0',
  `TabId` tinyint(1) unsigned NOT NULL default '0',
  `PlayerGuid` int(11) unsigned NOT NULL default '0',
  `ItemOrMoney` int(11) unsigned NOT NULL default '0',
  `ItemStackCount` tinyint(3) unsigned NOT NULL default '0',
  `DestTabId` tinyint(1) unsigned NOT NULL default '0',
  `TimeStamp` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildid`,`LogGuid`),
  KEY `guildid_key` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
