-- Query let convert characters DB from format
-- CMaNGOS Classic characters DB `required_z1794_s1350_11716_09_characters_auction` (copy-paste typo in quard name in fact) to
-- CMaNGOS TBC characters DB `required_s1350_11716_09_characters_mail`.

-- Expected that in case CMangos Classic characters DB changes it will updated for more up-to-date versions.
-- For targeted CMaNGOS Master characters DB you can after convertion apply CMaNGOS SQL updates in normal
-- way for update characters DB to more recent revision.

-- Note: ALWAYS DO BACKUP before use it. You will CAN NOT easy restore original DB state after tool use.

ALTER TABLE character_db_version CHANGE COLUMN `required_z1794_s1350_11716_09_characters_auction` `required_s1350_11716_09_characters_mail` bit;


-- MODIFID TABLES

ALTER TABLE characters
  ADD COLUMN `dungeon_difficulty` tinyint(1) unsigned NOT NULL DEFAULT '0' AFTER `map`,
  ADD COLUMN `arenaPoints` int(10) UNSIGNED NOT NULL default '0' AFTER `taxi_path`,
  ADD COLUMN `totalHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER `arenaPoints`,
  ADD COLUMN `todayHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER `totalHonorPoints`,
  ADD COLUMN `yesterdayHonorPoints` int(10) UNSIGNED NOT NULL default '0' AFTER `todayHonorPoints`,
  ADD COLUMN `totalKills` int(10) UNSIGNED NOT NULL default '0' AFTER `yesterdayHonorPoints`,
  ADD COLUMN `todayKills` smallint(5) UNSIGNED NOT NULL default '0' AFTER `totalKills`,
  ADD COLUMN `yesterdayKills` smallint(5) UNSIGNED NOT NULL default '0' AFTER `todayKills`,
  ADD COLUMN `chosenTitle` int(10) UNSIGNED NOT NULL default '0' AFTER `yesterdayKills`,
  DROP COLUMN `honor_highest_rank`,
  DROP COLUMN `honor_standing`,
  DROP COLUMN `stored_honor_rating`,
  DROP COLUMN `stored_dishonorable_kills`,
  DROP COLUMN `stored_honorable_kills`,
  ADD COLUMN `knownTitles` longtext AFTER `ammoId`;

UPDATE characters
  SET at_login = at_login | 4; -- reset talents at login

UPDATE characters              --  avoid client crashes at not existed item models
  SET equipmentCache = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ';

UPDATE characters
  SET knownTitles = '0 0 ';

ALTER TABLE character_social
  ADD COLUMN `note` varchar(48) NOT NULL DEFAULT '' COMMENT 'Friend Note' AFTER `flags`;


ALTER TABLE character_stats
  ADD COLUMN `spellCritPct` float UNSIGNED NOT NULL default '0' AFTER `rangedCritPct`,
  ADD COLUMN `spellPower` int(10) UNSIGNED NOT NULL default '0' AFTER `rangedAttackPower`;

ALTER TABLE groups
  ADD COLUMN `difficulty` tinyint(3) unsigned NOT NULL default '0' AFTER isRaid;

ALTER TABLE guild
  ADD COLUMN `BankMoney` bigint(20) unsigned NOT NULL default '0' AFTER `createdate`;

ALTER TABLE guild_member
  ADD COLUMN `BankResetTimeMoney` int(11) unsigned NOT NULL default '0' AFTER `offnote`,
  ADD COLUMN `BankRemMoney` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeMoney`,
  ADD COLUMN `BankResetTimeTab0` int(11) unsigned NOT NULL default '0' AFTER `BankRemMoney`,
  ADD COLUMN `BankRemSlotsTab0` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab0`,
  ADD COLUMN `BankResetTimeTab1` int(11) unsigned NOT NULL default '0' AFTER `BankRemSlotsTab0`,
  ADD COLUMN `BankRemSlotsTab1` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab1`,
  ADD COLUMN `BankResetTimeTab2` int(11) unsigned NOT NULL default '0' AFTER `BankRemSlotsTab1`,
  ADD COLUMN `BankRemSlotsTab2` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab2`,
  ADD COLUMN `BankResetTimeTab3` int(11) unsigned NOT NULL default '0' AFTER `BankRemSlotsTab2`,
  ADD COLUMN `BankRemSlotsTab3` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab3`,
  ADD COLUMN `BankResetTimeTab4` int(11) unsigned NOT NULL default '0' AFTER `BankRemSlotsTab3`,
  ADD COLUMN `BankRemSlotsTab4` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab4`,
  ADD COLUMN `BankResetTimeTab5` int(11) unsigned NOT NULL default '0' AFTER `BankRemSlotsTab4`,
  ADD COLUMN `BankRemSlotsTab5` int(11) unsigned NOT NULL default '0' AFTER `BankResetTimeTab5`;

ALTER TABLE guild_rank
  ADD COLUMN `BankMoneyPerDay` int(11) unsigned NOT NULL default '0' AFTER `rights`;

ALTER TABLE instance
  ADD COLUMN `difficulty` tinyint(1) unsigned NOT NULL default '0' AFTER `resettime`;

ALTER TABLE item_loot
  ADD COLUMN `suffix` int(11) unsigned NOT NULL default '0' AFTER `amount`;

ALTER TABLE petition
  ADD COLUMN `type` int(10) unsigned NOT NULL default '0' AFTER `name`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`ownerguid`,`type`);

UPDATE petition
  SET type = 9;  -- guild petition

ALTER TABLE petition_sign
  ADD COLUMN `type` int(10) unsigned NOT NULL default '0' AFTER `player_account`;

UPDATE petition_sign
  SET type = 9;  -- guild petition

ALTER TABLE saved_variables
  ADD COLUMN `NextArenaPointDistributionTime` bigint(40) UNSIGNED NOT NULL DEFAULT '0' AFTER `NextMaintenanceDate`,
  ADD COLUMN `NextDailyQuestResetTime` bigint(40) unsigned NOT NULL default '0' AFTER `NextArenaPointDistributionTime`;


DELETE FROM `character_spell_cooldown`;
DELETE FROM `pet_spell_cooldown`;
DELETE FROM `character_tutorial`;
DELETE FROM `character_aura`;
DELETE FROM `pet_aura`;


-- FIELD VALUES CONVERT

-- 0.6 -> 0.7 none

-- 0.7 -> 0.8

DELETE FROM `character_spell` WHERE `spell` = '23301';

-- 0.8 -> 0.9 none
-- 0.9 -> 0.10 none
-- 0.10 -> 0.11 none

UPDATE item_instance SET data= CONCAT(
  SUBSTRING_INDEX(SUBSTRING_INDEX(data,' ',22),' ',-22),' 0 0 0 0 0 0 0 0 0 0 0 0 ',
  SUBSTRING_INDEX(SUBSTRING_INDEX(data,' ',48),' ',-48+22))
WHERE SUBSTRING_INDEX(data,' ',48) = data AND SUBSTRING_INDEX(data,' ',48-1) <> data;

UPDATE item_instance SET data= CONCAT(
  SUBSTRING_INDEX(SUBSTRING_INDEX(data,' ',22),' ',-22),' 0 0 0 0 0 0 0 0 0 0 0 0 ',
  SUBSTRING_INDEX(SUBSTRING_INDEX(data,' ',106),' ',-106+22),' 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ')
WHERE SUBSTRING_INDEX(data,' ',106) = data AND SUBSTRING_INDEX(data,' ',106-1) <> data;

-- NEW TABLES with non critical info drop

DROP TABLE IF EXISTS `arena_team`;
CREATE TABLE `arena_team` (
  `arenateamid` int(10) unsigned NOT NULL default '0',
  `name` char(255) NOT NULL,
  `captainguid` int(10) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL default '0',
  `BackgroundColor` int(10) unsigned NOT NULL default '0',
  `EmblemStyle` int(10) unsigned NOT NULL default '0',
  `EmblemColor` int(10) unsigned NOT NULL default '0',
  `BorderStyle` int(10) unsigned NOT NULL default '0',
  `BorderColor` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`arenateamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `arena_team_member`;
CREATE TABLE `arena_team_member` (
  `arenateamid` int(10) unsigned NOT NULL default '0',
  `guid` int(10) unsigned NOT NULL default '0',
  `played_week` int(10) unsigned NOT NULL default '0',
  `wons_week` int(10) unsigned NOT NULL default '0',
  `played_season` int(10) unsigned NOT NULL default '0',
  `wons_season` int(10) unsigned NOT NULL default '0',
  `personal_rating` int(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY  (`arenateamid`,`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `arena_team_stats`;
CREATE TABLE `arena_team_stats` (
  `arenateamid` int(10) unsigned NOT NULL default '0',
  `rating` int(10) unsigned NOT NULL default '0',
  `games_week` int(10) unsigned NOT NULL default '0',
  `wins_week` int(10) unsigned NOT NULL default '0',
  `games_season` int(10) unsigned NOT NULL default '0',
  `wins_season` int(10) unsigned NOT NULL default '0',
  `rank` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`arenateamid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `character_declinedname`;
CREATE TABLE `character_declinedname` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `genitive` varchar(15) NOT NULL default '',
  `dative` varchar(15) NOT NULL default '',
  `accusative` varchar(15) NOT NULL default '',
  `instrumental` varchar(15) NOT NULL default '',
  `prepositional` varchar(15) NOT NULL default '',
  PRIMARY KEY  (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;


DROP TABLE IF EXISTS `character_pet_declinedname`;
CREATE TABLE `character_pet_declinedname` (
  `id` int(11) unsigned NOT NULL default '0',
  `owner` int(11) unsigned NOT NULL default '0',
  `genitive` varchar(12) NOT NULL default '',
  `dative` varchar(12) NOT NULL default '',
  `accusative` varchar(12) NOT NULL default '',
  `instrumental` varchar(12) NOT NULL default '',
  `prepositional` varchar(12) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY owner_key (`owner`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

DROP TABLE IF EXISTS `character_queststatus_daily`;
CREATE TABLE `character_queststatus_daily` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `quest` int(11) unsigned NOT NULL default '0' COMMENT 'Quest Identifier',
  PRIMARY KEY  (`guid`,`quest`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';


DROP TABLE IF EXISTS `guild_bank_eventlog`;
CREATE TABLE `guild_bank_eventlog` (
  `guildid` int(11) unsigned NOT NULL default '0' COMMENT 'Guild Identificator',
  `LogGuid` int(11) unsigned NOT NULL default '0' COMMENT 'Log record identificator - auxiliary column',
  `TabId` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Guild bank TabId',
  `EventType` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Event type',
  `PlayerGuid` int(11) unsigned NOT NULL default '0',
  `ItemOrMoney` int(11) unsigned NOT NULL default '0',
  `ItemStackCount` tinyint(3) unsigned NOT NULL default '0',
  `DestTabId` tinyint(1) unsigned NOT NULL default '0' COMMENT 'Destination Tab Id',
  `TimeStamp` bigint(20) unsigned NOT NULL default '0' COMMENT 'Event UNIX time',
  PRIMARY KEY  (`guildid`,`LogGuid`,`TabId`),
  KEY `guildid_key` (`guildid`),
  INDEX `Idx_PlayerGuid`(`PlayerGuid`),
  INDEX `Idx_LogGuid`(`LogGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `guild_bank_item`;
CREATE TABLE `guild_bank_item` (
  `guildid` int(11) unsigned NOT NULL default '0',
  `TabId` tinyint(1) unsigned NOT NULL default '0',
  `SlotId` tinyint(3) unsigned NOT NULL default '0',
  `item_guid` int(11) unsigned NOT NULL default '0',
  `item_entry` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildid`,`tabid`,`slotid`),
  KEY `guildid_key` (`guildid`),
  INDEX `Idx_item_guid`(`item_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `guild_bank_right`;
CREATE TABLE `guild_bank_right` (
  `guildid` int(11) unsigned NOT NULL default '0',
  `TabId` tinyint(1) unsigned NOT NULL default '0',
  `rid` int(11) unsigned NOT NULL default '0',
  `gbright` tinyint(3) unsigned NOT NULL default '0',
  `SlotPerDay` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`guildid`,`TabId`,`rid`),
  KEY `guildid_key` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `guild_bank_tab`;
CREATE TABLE `guild_bank_tab` (
  `guildid` int(11) unsigned NOT NULL default '0',
  `TabId` tinyint(1) unsigned NOT NULL default '0',
  `TabName` varchar(100) NOT NULL default '',
  `TabIcon` varchar(100) NOT NULL default '',
  `TabText` text,
  PRIMARY KEY  (`guildid`,`TabId`),
  KEY `guildid_key` (`guildid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- REMOVED TABLES with non critical info drop

DROP TABLE IF EXISTS `character_honor_cp`;
