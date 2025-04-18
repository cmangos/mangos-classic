-- MySQL dump 10.13
--
-- Host: localhost    Database: mangos
-- ------------------------------------------------------
-- Server version   5.5.32

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
-- Table structure for table `db_version`
--

DROP TABLE IF EXISTS `db_version`;
CREATE TABLE `db_version` (
  `version` varchar(120) DEFAULT NULL,
  `creature_ai_version` varchar(120) DEFAULT NULL,
  `required_z2826_01_mangos_spawn_group_squad` bit(1) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Used DB version notes';

--
-- Dumping data for table `db_version`
--

LOCK TABLES `db_version` WRITE;
/*!40000 ALTER TABLE `db_version` DISABLE KEYS */;
INSERT INTO `db_version` VALUES
('Mangos default database.','Creature EventAI not provided.',NULL);
/*!40000 ALTER TABLE `db_version` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `areatrigger_involvedrelation`
--

DROP TABLE IF EXISTS `areatrigger_involvedrelation`;
CREATE TABLE `areatrigger_involvedrelation` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_involvedrelation`
--

LOCK TABLES `areatrigger_involvedrelation` WRITE;
/*!40000 ALTER TABLE `areatrigger_involvedrelation` DISABLE KEYS */;
/*!40000 ALTER TABLE `areatrigger_involvedrelation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `areatrigger_tavern`
--

DROP TABLE IF EXISTS `areatrigger_tavern`;
CREATE TABLE `areatrigger_tavern` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `name` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_tavern`
--

LOCK TABLES `areatrigger_tavern` WRITE;
/*!40000 ALTER TABLE `areatrigger_tavern` DISABLE KEYS */;
/*!40000 ALTER TABLE `areatrigger_tavern` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `areatrigger_teleport`
--

DROP TABLE IF EXISTS `areatrigger_teleport`;
CREATE TABLE `areatrigger_teleport` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `name` text,
  `required_level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `required_item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `required_item2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `required_quest_done` int(11) unsigned NOT NULL DEFAULT '0',
  `target_map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `target_position_x` float NOT NULL DEFAULT '0',
  `target_position_y` float NOT NULL DEFAULT '0',
  `target_position_z` float NOT NULL DEFAULT '0',
  `target_orientation` float NOT NULL DEFAULT '0',
  `condition_id` INT(11) unsigned NOT NULL default '0',
  `status_failed_text` text,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Trigger System';

--
-- Dumping data for table `areatrigger_teleport`
--

LOCK TABLES `areatrigger_teleport` WRITE;
/*!40000 ALTER TABLE `areatrigger_teleport` DISABLE KEYS */;
/*!40000 ALTER TABLE `areatrigger_teleport` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `battleground_events`
--

DROP TABLE IF EXISTS `battleground_events`;
CREATE TABLE `battleground_events` (
  `map` smallint(5) NOT NULL,
  `event1` tinyint(3) unsigned NOT NULL,
  `event2` tinyint(3) unsigned NOT NULL,
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`map`,`event1`,`event2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `battleground_events`
--

LOCK TABLES `battleground_events` WRITE;
/*!40000 ALTER TABLE `battleground_events` DISABLE KEYS */;
INSERT INTO `battleground_events` VALUES
(489,0,0,'Alliance Flag'),
(489,1,0,'Horde Flag'),
(489,2,0,'Spirit Guides'),
(489,254,0,'Doors'),
(529,0,0,'Stables - neutral'),
(529,0,1,'Stables - alliance contested'),
(529,0,2,'Stables - horde contested'),
(529,0,3,'Stables - alliance owned'),
(529,0,4,'Stables - horde owned'),
(529,1,0,'Blacksmith - neutral'),
(529,1,1,'Blacksmith - alliance contested'),
(529,1,2,'Blacksmith - horde contested'),
(529,1,3,'Blacksmith - alliance owned'),
(529,1,4,'Blacksmith - horde owned'),
(529,2,0,'Farm - neutral'),
(529,2,1,'Farm - alliance contested'),
(529,2,2,'Farm - horde contested'),
(529,2,3,'Farm - alliance owned'),
(529,2,4,'Farm - horde owned'),
(529,3,0,'Lumber Mill - neutral'),
(529,3,1,'Lumber Mill - alliance contested'),
(529,3,2,'Lumber Mill - horde contested'),
(529,3,3,'Lumber Mill - alliance owned'),
(529,3,4,'Lumber Mill - horde owned'),
(529,4,0,'Gold Mine - neutral'),
(529,4,1,'Gold Mine - alliance contested'),
(529,4,2,'Gold Mine - horde contested'),
(529,4,3,'Gold Mine - alliance owned'),
(529,4,4,'Gold Mine - horde owned'),
(529,254,0,'doors'),
(30,254,0,'Doors'),
(30,0,0,'Firstaid Station - Alliance assaulted'),
(30,0,1,'Firstaid Station - ALliance control'),
(30,0,2,'Firstaid Station - Horde assaulted'),
(30,0,3,'Firstaid Station - Horde control'),
(30,1,0,'Stormpike Grave - Alliance assaulted'),
(30,1,1,'Stormpike Grave - ALliance control'),
(30,1,2,'Stormpike Grave - Horde assaulted'),
(30,1,3,'Stormpike Grave - Horde control'),
(30,2,0,'Stoneheart Grave - Alliance assaulted'),
(30,2,1,'Stoneheart Grave - ALliance control'),
(30,2,2,'Stoneheart Grave - Horde assaulted'),
(30,2,3,'Stoneheart Grave - Horde control'),
(30,3,0,'Snowfall Grave - Alliance assaulted'),
(30,3,1,'Snowfall Grave - ALliance control'),
(30,3,2,'Snowfall Grave - Horde assaulted'),
(30,3,3,'Snowfall Grave - Horde control'),
(30,3,5,'Snowfall Grave - Neutral control'),
(30,4,0,'Iceblood Grave - Alliance assaulted'),
(30,4,1,'Iceblood Grave - ALliance control'),
(30,4,2,'Iceblood Grave - Horde assaulted'),
(30,4,3,'Iceblood Grave - Horde control'),
(30,5,0,'Frostwolf Grave - Alliance assaulted'),
(30,5,1,'Frostwolf Grave - ALliance control'),
(30,5,2,'Frostwolf Grave - Horde assaulted'),
(30,5,3,'Frostwolf Grave - Horde control'),
(30,6,0,'Frostwolf Hut - Alliance assaulted'),
(30,6,1,'Frostwolf Hut - ALliance control'),
(30,6,2,'Frostwolf Hut - Horde assaulted'),
(30,6,3,'Frostwolf Hut - Horde control'),
(30,7,1,'Dunbaldar South - ALliance control'),
(30,7,2,'Dunbaldar South - Horde assaulted'),
(30,7,3,'Dunbaldar South - Horde control'),
(30,8,1,'Dunbaldar North - ALliance control'),
(30,8,2,'Dunbaldar North - Horde assaulted'),
(30,8,3,'Dunbaldar North - Horde control'),
(30,9,1,'Icewing Bunker - ALliance control'),
(30,9,2,'Icewing Bunker - Horde assaulted'),
(30,9,3,'Icewing Bunker - Horde control'),
(30,10,1,'Stoneheart Bunker - ALliance control'),
(30,10,2,'Stoneheart Bunker - Horde assaulted'),
(30,10,3,'Stoneheart Bunker - Horde control'),
(30,11,0,'Iceblood Tower - Alliance assaulted'),
(30,11,1,'Iceblood Tower - ALliance control'),
(30,11,3,'Iceblood Tower - Horde control'),
(30,12,0,'Tower Point - Alliance assaulted'),
(30,12,1,'Tower Point - ALliance control'),
(30,12,3,'Tower Point - Horde control'),
(30,13,0,'Frostwolf east Tower - Alliance assaulted'),
(30,13,1,'Frostwolf east Tower - ALliance control'),
(30,13,3,'Frostwolf east Tower - Horde control'),
(30,14,0,'Frostwolf west Tower - Alliance assaulted'),
(30,14,1,'Frostwolf west Tower - ALliance control'),
(30,14,3,'Frostwolf west Tower - Horde control'),
(30,15,0,'Firstaid Station - Alliance Defender Quest0'),
(30,15,1,'Firstaid Station - Alliance Defender Quest1'),
(30,15,2,'Firstaid Station - Alliance Defender Quest2'),
(30,15,3,'Firstaid Station - Alliance Defender Quest3'),
(30,15,4,'Firstaid Station - Horde Defender Quest0'),
(30,15,5,'Firstaid Station - Horde Defender Quest1'),
(30,15,6,'Firstaid Station - Horde Defender Quest2'),
(30,15,7,'Firstaid Station - Horde Defender Quest3'),
(30,16,0,'Stormpike Grave - Alliance Defender Quest0'),
(30,16,1,'Stormpike Grave - Alliance Defender Quest1'),
(30,16,2,'Stormpike Grave - Alliance Defender Quest2'),
(30,16,3,'Stormpike Grave - Alliance Defender Quest3'),
(30,16,4,'Stormpike Grave - Horde Defender Quest0'),
(30,16,5,'Stormpike Grave - Horde Defender Quest1'),
(30,16,6,'Stormpike Grave - Horde Defender Quest2'),
(30,16,7,'Stormpike Grave - Horde Defender Quest3'),
(30,17,0,'Stoneheart Grave - Alliance Defender Quest0'),
(30,17,1,'Stoneheart Grave - Alliance Defender Quest1'),
(30,17,2,'Stoneheart Grave - Alliance Defender Quest2'),
(30,17,3,'Stoneheart Grave - Alliance Defender Quest3'),
(30,17,4,'Stoneheart Grave - Horde Defender Quest0'),
(30,17,5,'Stoneheart Grave - Horde Defender Quest1'),
(30,17,6,'Stoneheart Grave - Horde Defender Quest2'),
(30,17,7,'Stoneheart Grave - Horde Defender Quest3'),
(30,18,0,'Snowfall Grave - Alliance Defender Quest0'),
(30,18,1,'Snowfall Grave - Alliance Defender Quest1'),
(30,18,2,'Snowfall Grave - Alliance Defender Quest2'),
(30,18,3,'Snowfall Grave - Alliance Defender Quest3'),
(30,18,4,'Snowfall Grave - Horde Defender Quest0'),
(30,18,5,'Snowfall Grave - Horde Defender Quest1'),
(30,18,6,'Snowfall Grave - Horde Defender Quest2'),
(30,18,7,'Snowfall Grave - Horde Defender Quest3'),
(30,19,0,'Iceblood Grave - Alliance Defender Quest0'),
(30,19,1,'Iceblood Grave - Alliance Defender Quest1'),
(30,19,2,'Iceblood Grave - Alliance Defender Quest2'),
(30,19,3,'Iceblood Grave - Alliance Defender Quest3'),
(30,19,4,'Iceblood Grave - Horde Defender Quest0'),
(30,19,5,'Iceblood Grave - Horde Defender Quest1'),
(30,19,6,'Iceblood Grave - Horde Defender Quest2'),
(30,19,7,'Iceblood Grave - Horde Defender Quest3'),
(30,20,0,'Frostwolf Grave - Alliance Defender Quest0'),
(30,20,1,'Frostwolf Grave - Alliance Defender Quest1'),
(30,20,2,'Frostwolf Grave - Alliance Defender Quest2'),
(30,20,3,'Frostwolf Grave - Alliance Defender Quest3'),
(30,20,4,'Frostwolf Grave - Horde Defender Quest0'),
(30,20,5,'Frostwolf Grave - Horde Defender Quest1'),
(30,20,6,'Frostwolf Grave - Horde Defender Quest2'),
(30,20,7,'Frostwolf Grave - Horde Defender Quest3'),
(30,21,0,'Frostwolf Hut - Alliance Defender Quest0'),
(30,21,1,'Frostwolf Hut - Alliance Defender Quest1'),
(30,21,2,'Frostwolf Hut - Alliance Defender Quest2'),
(30,21,3,'Frostwolf Hut - Alliance Defender Quest3'),
(30,21,4,'Frostwolf Hut - Horde Defender Quest0'),
(30,21,5,'Frostwolf Hut - Horde Defender Quest1'),
(30,21,6,'Frostwolf Hut - Horde Defender Quest2'),
(30,21,7,'Frostwolf Hut - Horde Defender Quest3'),
(30,46,0,'North Mine - Alliance Boss'),
(30,46,1,'North Mine - Horde Boss'),
(30,46,2,'North Mine - Neutral Boss'),
(30,47,0,'South Mine - Alliance Boss'),
(30,47,1,'South Mine - Horde Boss'),
(30,47,2,'South Mine - Neutral Boss'),
(30,48,0,'Alliance Captain'),
(30,49,0,'Horde Captain'),
(30,50,0,'North Mine - Alliance Control'),
(30,50,1,'North Mine - Horde Control'),
(30,50,2,'North Mine - Neutral Control'),
(30,51,0,'South Mine - Alliance Control'),
(30,51,1,'South Mine - Horde Control'),
(30,51,2,'South Mine - Neutral Control'),
(30,52,0,'Alliance Marshal - Dunbaldar South'),
(30,53,0,'Alliance Marshal - Dunbaldar North'),
(30,54,0,'Alliance Marshal - Icewing Bunker'),
(30,55,0,'Alliance Marshal - Stoneheart Bunker'),
(30,56,0,'Horde Marshal - Iceblood Tower'),
(30,57,0,'Horde Marshal - Towerpoint'),
(30,58,0,'Horde Marshal - East Frostwolf Tower'),
(30,59,0,'Horde Marshal - West Frostwolf Tower'),
(30,60,0,'Herald - that guy who yells all the time ;)'),
(30,61,0,'Alliance - Boss'),
(30,62,0,'Horde - Boss'),
(30,63,0,'Alliance - Captain Dead'),
(30,64,0,'Horde - Captain Dead');
/*!40000 ALTER TABLE `battleground_events` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `battleground_template`
--

DROP TABLE IF EXISTS `battleground_template`;
CREATE TABLE `battleground_template` (
  `id` mediumint(8) unsigned NOT NULL,
  `MinPlayersPerTeam` smallint(5) unsigned NOT NULL DEFAULT '0',
  `MaxPlayersPerTeam` smallint(5) unsigned NOT NULL DEFAULT '0',
  `MinLvl` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MaxLvl` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AllianceStartLoc` mediumint(8) unsigned NOT NULL,
  `HordeStartLoc` mediumint(8) unsigned NOT NULL,
  `StartMaxDist` float NOT NULL,
  `PlayerSkinReflootId` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'reference_loot_template entry',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `battleground_template`
--

LOCK TABLES `battleground_template` WRITE;
/*!40000 ALTER TABLE `battleground_template` DISABLE KEYS */;
INSERT INTO `battleground_template` VALUES
(1,20,40,51,60,611,610,100,0),
(2,5,10,10,60,769,770,75,0),
(3,8,15,20,60,890,889,75,0);
/*!40000 ALTER TABLE `battleground_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `battlemaster_entry`
--

DROP TABLE IF EXISTS `battlemaster_entry`;
CREATE TABLE `battlemaster_entry` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Entry of a creature',
  `bg_template` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Battleground template id',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `battlemaster_entry`
--

LOCK TABLES `battlemaster_entry` WRITE;
/*!40000 ALTER TABLE `battlemaster_entry` DISABLE KEYS */;
/*!40000 ALTER TABLE `battlemaster_entry` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `command`
--

DROP TABLE IF EXISTS `command`;
CREATE TABLE `command` (
  `name` varchar(50) NOT NULL DEFAULT '',
  `security` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `help` longtext,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Chat System';

--
-- Dumping data for table `command`
--

LOCK TABLES `command` WRITE;
/*!40000 ALTER TABLE `command` DISABLE KEYS */;
INSERT INTO `command` VALUES
('account',0,'Syntax: .account\r\n\r\nDisplay the access level of your account.'),
('account characters',3,'Syntax: .account characters [#accountId|$accountName]\r\n\r\nShow list all characters for account selected by provided #accountId or $accountName, or for selected player in game.'),
('account create',4,'Syntax: .account create $account $password\r\n\r\nCreate account and set password to it.'),
('account delete',4,'Syntax: .account delete $account\r\n\r\nDelete account with all characters.'),
('account lock',0,'Syntax: .account lock [on|off]\r\n\r\nAllow login from account only from current used IP or remove this requirement.'),
('account onlinelist',4,'Syntax: .account onlinelist\r\n\r\nShow list of online accounts.'),
('account password',0,'Syntax: .account password $old_password $new_password $new_password\r\n\r\nChange your account password.'),
('account set addon',3,'Syntax: .account set addon [#accountId|$accountName] #addon\r\n\r\nSet user (possible targeted) expansion addon level allowed. Addon values: 0 - normal, 1 - tbc, 2 - wotlk.'),
('account set gmlevel',4,'Syntax: .account set gmlevel [#accountId|$accountName] #level\r\n\r\nSet the security level for targeted player (can\'t be used at self) or for #accountId or $accountName to a level of #level.\r\n\r\n#level may range from 0 to 3.'),
('account set password',4,'Syntax: .account set password (#accountId|$accountName) $password $password\r\n\r\nSet password for account.'),
('additem',3,'Syntax: .additem #itemid/[#itemname]/#shift-click-item-link #itemcount\r\n\r\nAdds the specified number of items of id #itemid (or exact (!) name $itemname in brackets, or link created by shift-click at item in inventory or recipe) to your or selected character inventory. If #itemcount is omitted, only one item will be added.\r\n.'),
('additemset',3,'Syntax: .additemset #itemsetid\r\n\r\nAdd items from itemset of id #itemsetid to your or selected character inventory. Will add by one example each item from itemset.'),
('announce',1,'Syntax: .announce $MessageToBroadcast\r\n\r\nSend a global message to all players online in chat log.'),
('auction',3,'Syntax: .auction\r\n\r\nShow your team auction store.'),
('auction alliance',3,'Syntax: .auction alliance\r\n\r\nShow alliance auction store independent from your team.'),
('auction goblin',3,'Syntax: .auction goblin\r\n\r\nShow goblin auction store common for all teams.'),
('auction horde',3,'Syntax: .auction horde\r\n\r\nShow horde auction store independent from your team.'),
('auction item',3,'Syntax: .auction item (alliance|horde|goblin) #itemid[:#itemcount] [[[#minbid] #buyout] [short|long|verylong]\r\n\r\nAdd new item to an specific auction house at short|long|verylong periods. Periods are the same as the ones in the in-game dialog. Created auction does not have an owner.'),
('aura',3,'Syntax: .aura #spellid\r\n\r\nAdd the aura from spell #spellid to the selected Unit.'),
('ban account',3,'Syntax: .ban account $Name $bantime $reason\r\nBan account kick player.\r\n$bantime: negative value leads to permban, otherwise use a timestring like \"4d20h3s\".'),
('ban character',3,'Syntax: .ban character $Name $bantime $reason\r\nBan account and kick player.\r\n$bantime: negative value leads to permban, otherwise use a timestring like \"4d20h3s\".'),
('ban ip',3,'Syntax: .ban ip $Ip $bantime $reason\r\nBan IP.\r\n$bantime: negative value leads to permban, otherwise use a timestring like \"4d20h3s\".'),
('baninfo account',3,'Syntax: .baninfo account $accountid\r\nWatch full information about a specific ban.'),
('baninfo character',3,'Syntax: .baninfo character $charactername \r\nWatch full information about a specific ban.'),
('baninfo ip',3,'Syntax: .baninfo ip $ip\r\nWatch full information about a specific ban.'),
('bank',3,'Syntax: .bank\r\n\r\nShow your bank inventory.'),
('banlist account',3,'Syntax: .banlist account [$Name]\r\nSearches the banlist for a account name pattern or show full list account bans.'),
('banlist character',3,'Syntax: .banlist character $Name\r\nSearches the banlist for a character name pattern. Pattern required.'),
('banlist ip',3,'Syntax: .banlist ip [$Ip]\r\nSearches the banlist for a IP pattern or show full list of IP bans.'),
('cast',3,'Syntax: .cast #spellid [triggered]\r\n  Cast #spellid to selected target. If no target selected cast to self. If \'trigered\' or part provided then spell casted with triggered flag.'),
('cast back',3,'Syntax: .cast back #spellid [triggered]\r\n  Selected target will cast #spellid to your character. If \'trigered\' or part provided then spell casted with triggered flag.'),
('cast dist',3,'Syntax: .cast dist #spellid [#dist [triggered]]\r\n  You will cast spell to pint at distance #dist. If \'trigered\' or part provided then spell casted with triggered flag. Not all spells can be casted as area spells.'),
('cast self',3,'Syntax: .cast self #spellid [triggered]\r\nCast #spellid by target at target itself. If \'trigered\' or part provided then spell casted with triggered flag.'),
('cast target',3,'Syntax: .cast target #spellid [triggered]\r\n  Selected target will cast #spellid to his victim. If \'trigered\' or part provided then spell casted with triggered flag.'),
('channel list',1,'Syntax: .channel list [#max] [static]\r\n\r\nShow list of custom channels with amounts of players joined.'),
('channel static',1,'Syntax: .channel static $channelname on|off\r\n\r\nEnable or disable static mode for a custom channel with name $channelname. Static custom channel upon conversion acquires a set of properties identical to global channes.'),
('character deleted delete',4,'Syntax: .character deleted delete #guid|$name\r\n\r\nCompletely deletes the selected characters.\r\nIf $name is supplied, only characters with that string in their name will be deleted, if #guid is supplied, only the character with that GUID will be deleted.'),
('character deleted list',4,'Syntax: .character deleted list [#guid|$name]\r\n\r\nShows a list with all deleted characters.\r\nIf $name is supplied, only characters with that string in their name will be selected, if #guid is supplied, only the character with that GUID will be selected.'),
('character deleted old',4,'Syntax: .character deleted old [#keepDays]\r\n\r\nCompletely deletes all characters with deleted time longer #keepDays. If #keepDays not provided the  used value from mangosd.conf option \'CharDelete.KeepDays\'. If referenced config option disabled (use 0 value) then command can\'t be used without #keepDays.'),
('character deleted restore',3,'Syntax: .character deleted restore #guid|$name [$newname] [#new account]\r\n\r\nRestores deleted characters.\r\nIf $name is supplied, only characters with that string in their name will be restored, if $guid is supplied, only the character with that GUID will be restored.\r\nIf $newname is set, the character will be restored with that name instead of the original one. If #newaccount is set, the character will be restored to specific account character list. This works only with one character!'),
('character erase',4,'Syntax: .character erase $name\r\n\r\nDelete character $name. Character finally deleted in case any deleting options.'),
('character level',3,'Syntax: .character level [$playername] [#level]\r\n\r\nSet the level of character with $playername (or the selected if not name provided) by #numberoflevels Or +1 if no #numberoflevels provided). If #numberoflevels is omitted, the level will be increase by 1. If #numberoflevels is 0, the same level will be restarted. If no character is selected and name not provided, increase your level. Command can be used for offline character. All stats and dependent values recalculated. At level decrease talents can be reset if need. Also at level decrease equipped items with greater level requirement can be lost.'),
('character rename',2,'Syntax: .character rename [$name]\r\n\r\nMark selected in game or by $name in command character for rename at next login.'),
('character reputation',2,'Syntax: .character reputation [$player_name]\r\n\r\nShow reputation information for selected player or player find by $player_name.'),
('combat stop',2,'Syntax: .combatstop [$playername]\r\nStop combat for selected character. If selected non-player then command applied to self. If $playername provided then attempt applied to online player $playername.'),
('commands',0,'Syntax: .commands\r\n\r\nDisplay a list of available commands for your account level.'),
('cooldown list',3,'Syntax: .cooldown list  Active cooldown from selected unit.'),
('cooldown clear',3,'Syntax: .cooldown clear [spell id] Remove cooldown from selected unit.'),
('cooldown clearclientside',3,'Syntax: .cooldown clearclientside  Clear all cooldown client side only.'),
('damage',3,'Syntax: .damage $damage_amount [$school [$spellid]]\r\n\r\nApply $damage to target. If not $school and $spellid provided then this flat clean melee damage without any modifiers. If $school provided then damage modified by armor reduction (if school physical), and target absorbing modifiers and result applied as melee damage to target. If spell provided then damage modified and applied as spell damage. $spellid can be shift-link.'),
('debug anim',2,'Syntax: .debug anim #emoteid\r\n\r\nPlay emote #emoteid for your character.'),
('debug areatriggers', 1, 'Syntax: .debug areatriggers\n\nToggle debug mode for areatriggers. In debug mode GM will be notified if reaching an areatrigger.'),
('debug bg',3,'Syntax: .debug bg\r\n\r\nToggle debug mode for battlegrounds. In debug mode GM can start battleground with single player.'),
('debug dbscript',3,'.debug dbscript\r\n\r\nStarts dbscript type param0 id param1 from player(source) to selected(target)'),
('debug dbscripttargeted',3,'.debug dbscript\r\n\r\nStarts dbscript type param0 id param1 from selected(source) to param2 dbguid(target creature)'),
('debug dbscriptsourced',3,'.debug dbscript\r\n\r\nStarts dbscript type param0 id param1 from param2 dbguid(source creature) to selected(target)'),
('debug dbscriptguided',3,'.debug dbscript\r\n\r\nStarts dbscript type param0 id param1 from param2 dbguid(source creature) to param3 dbguid(target creature)'),
('debug getitemvalue',3,'Syntax: .debug getitemvalue #itemguid #field [int|hex|bit|float]\r\n\r\nGet the field #field of the item #itemguid in your inventroy.\r\n\r\nUse type arg for set output format: int (decimal number), hex (hex value), bit (bitstring), float. By default use integer output.'),
('debug getvaluebyindex', 3, 'Syntax: .debug getvaluebyindex #field [int|hex|bit|float]\r\n\r\nGet the field index #field (integer) of the selected target. If no target is selected, get the content of your field.\r\n\r\nUse type arg for set output format: int (decimal number), hex (hex value), bit (bitstring), float. By default use integer output.'),
('debug getvaluebyname', 3, 'Syntax: .debug getvaluebyname #field [int|hex|bit|float]\r\n\r\nGet the field name #field (string) of the selected target. If no target is selected, get the content of your field.\r\n\r\nUse type arg for set output format: int (decimal number), hex (hex value), bit (bitstring), float. By default use integer output.'),
('debug moditemvalue',3,'Syntax: .debug moditemvalue #guid #field [int|float| &= | |= | &=~ ] #value\r\n\r\nModify the field #field of the item #itemguid in your inventroy by value #value. \r\n\r\nUse type arg for set mode of modification: int (normal add/subtract #value as decimal number), float (add/subtract #value as float number), &= (bit and, set to 0 all bits in value if it not set to 1 in #value as hex number), |= (bit or, set to 1 all bits in value if it set to 1 in #value as hex number), &=~ (bit and not, set to 0 all bits in value if it set to 1 in #value as hex number). By default expect integer add/subtract.'),
('debug modvalue',3,'Syntax: .debug modvalue #field [int|float| &= | |= | &=~ ] #value\r\n\r\nModify the field #field of the selected target by value #value. If no target is selected, set the content of your field.\r\n\r\nUse type arg for set mode of modification: int (normal add/subtract #value as decimal number), float (add/subtract #value as float number), &= (bit and, set to 0 all bits in value if it not set to 1 in #value as hex number), |= (bit or, set to 1 all bits in value if it set to 1 in #value as hex number), &=~ (bit and not, set to 0 all bits in value if it set to 1 in #value as hex number). By default expect integer add/subtract.'),
('debug play cinematic',1,'Syntax: .debug play cinematic #cinematicid\r\n\r\nPlay cinematic #cinematicid for you. You stay at place while your mind fly.\r\n'),
('debug play sound',1,'Syntax: .debug play sound #soundid\r\n\r\nPlay sound with #soundid.\r\nSound will be play only for you. Other players do not hear this.\r\nWarning: client may have more 5000 sounds...'),
('debug setitemvalue',3,'Syntax: .debug setitemvalue #guid #field [int|hex|bit|float] #value\r\n\r\nSet the field #field of the item #itemguid in your inventroy to value #value.\r\n\r\nUse type arg for set input format: int (decimal number), hex (hex value), bit (bitstring), float. By default expect integer input format.'),
('debug setvaluebyindex', 3, 'Syntax: .debug setvaluebyindex #field [int|hex|bit|float] #value\r\n\r\nSet the field index #field (integer) of the selected target to value #value. If no target is selected, set the content of your field.\r\n\r\nUse type arg for set input format: int (decimal number), hex (hex value), bit (bitstring), float. By default expect integer input format.'),
('debug setvaluebyname', 3, 'Syntax: .debug setvaluebyname #field [int|hex|bit|float] #value\r\n\r\nSet the field name #field (string) of the selected target to value #value. If no target is selected, set the content of your field.\r\n\r\nUse type arg for set input format: int (decimal number), hex (hex value), bit (bitstring), float. By default expect integer input format.'),
('debug spellcoefs',3,'Syntax: .debug spellcoefs #spellid\r\n\r\nShow default calculated and DB stored coefficients for direct/dot heal/damage.'),
('debug spellmods',3,'Syntax: .debug spellmods (flat|pct) #spellMaskBitIndex #spellModOp #value\r\n\r\nSet at client side spellmod affect for spell that have bit set with index #spellMaskBitIndex in spell family mask for values dependent from spellmod #spellModOp to #value.'),
('debug taxi',3,'Syntax: .debug taxi\r\n\r\nToggle debug mode for taxi flights. In debug mode GM receive additional on-screen information during taxi flights.'),
('demorph',2,'Syntax: .demorph\r\n\r\nDemorph the selected player.'),
('die',3,'Syntax: .die\r\n\r\nKill the selected player. If no player is selected, it will kill you.'),
('dismount',0,'Syntax: .dismount\r\n\r\nDismount you, if you are mounted.'),
('distance',3,'Syntax: .distance [$name/$link]\r\n\r\nDisplay the distance from your character to the selected creature/player, or player with name $name, or player/creature/gameobject pointed to shift-link with guid.'),
('event',2,'Syntax: .event #event_id\r\nShow details about event with #event_id.'),
('event list',2,'Syntax: .event list\r\nShow list of currently active events.\r\nShow list of all events'),
('event start',2,'Syntax: .event start #event_id\r\nStart event #event_id. Set start time for event to current moment (change not saved in DB).'),
('event stop',2,'Syntax: .event stop #event_id\r\nStop event #event_id. Set start time for event to time in past that make current moment is event stop time (change not saved in DB).'),
('explorecheat',3,'Syntax: .explorecheat #flag\r\n\r\nReveal  or hide all maps for the selected player. If no player is selected, hide or reveal maps to you.\r\n\r\nUse a #flag of value 1 to reveal, use a #flag value of 0 to hide all maps.'),
('gm',3,'Syntax: .gm [on/off]\r\n\r\nEnable or Disable in game GM MODE or show current state of on/off not provided.'),
('gm chat',3,'Syntax: .gm chat [on/off]\r\n\r\nEnable or disable chat GM MODE (show gm badge in messages) or show current state of on/off not provided.'),
('gm fly',3,'Syntax: .gm fly [on/off]\r\nEnable/disable gm fly mode.'),
('gm ingame',3,'Syntax: .gm ingame\r\n\r\nDisplay a list of available in game Game Masters.'),
('gm list',3,'Syntax: .gm list\r\n\r\nDisplay a list of all Game Masters accounts and security levels.'),
('gm mountup',3,'Syntax: .gm mountup [fast|slow|#displayid|target]\r\n\r\nIf #displayid is provided, visually mounts your character on a provided creature likeness. If your target is a creature and corresponding arg is provided, visually mounts your character on the likeness of targeted creature. In other cases this command mounts your character on a random unusual land mount according to provided or omitted args.'),
('gm setview',3,'Syntax: .gm setview\r\n\r\nSet farsight view on selected unit. Select yourself to set view back.'),
('gm visible',3,'Syntax: .gm visible on/off\r\n\r\nOutput current visibility state or make GM visible(on) and invisible(off) for other players.'),
('go',1,'Syntax: .go  [$playername|pointlink|#x #y #z [#mapid]]\r\nTeleport your character to point with coordinates of player $playername, or coordinates of one from shift-link types: player, tele, taxinode, creature/creature_entry, gameobject/gameobject_entry, or explicit #x #y #z #mapid coordinates.'),
('go creature',1,'Syntax: .go creature (#creature_guid|$creature_name|id #creature_id)\r\nTeleport your character to creature with guid #creature_guid, or teleport your character to creature with name including as part $creature_name substring, or teleport your character to a creature that was spawned from the template with this entry #creature_id.'),
('go graveyard',1,'Syntax: .go graveyard #graveyardId\r\n Teleport to graveyard with the graveyardId specified.'),
('go grid',1,'Syntax: .go grid #gridX #gridY [#mapId]\r\n\r\nTeleport the gm to center of grid with provided indexes at map #mapId (or current map if it not provided).'),
('go object',1,'Syntax: .go object (#gameobject_guid|$gameobject_name|id #gameobject_id)\r\nTeleport your character to gameobject with guid #gameobject_guid, or teleport your character to gameobject with name including as part $gameobject_name substring, or teleport your character to a gameobject that was spawned from the template with this entry #gameobject_id.'),
('go taxinode',1,'Syntax: .go taxinode #taxinode\r\n\r\nTeleport player to taxinode coordinates. You can look up zone using .lookup taxinode $namepart'),
('go trigger',1,'Syntax: .go trigger (#trigger_id|$trigger_shift-link|$trigger_target_shift-link) [target]\r\n\r\nTeleport your character to areatrigger with id #trigger_id or trigger id associated with shift-link. If additional arg \"target\" provided then character will teleported to areatrigger target point.'),
('go warp', 1,'Syntax: .go warp #axis #value\n\nTeleports the user by the specified value along the specified axis.\nUse a positive value to move forward the axis, and a negative value to move backward the axis.\nValid axis are x (+forward/-backward), y (+right/-left), z (+above/-below), o (orientation, value is specified in degrees).'),
('go xy',1,'Syntax: .go xy #x #y [#mapid]\r\n\r\nTeleport player to point with (#x,#y) coordinates at ground(water) level at map #mapid or same map if #mapid not provided.'),
('go xyz',1,'Syntax: .go xyz #x #y #z [#mapid]\r\n\r\nTeleport player to point with (#x,#y,#z) coordinates at ground(water) level at map #mapid or same map if #mapid not provided.'),
('go zonexy',1,'Syntax: .go zonexy #x #y [#zone]\r\n\r\nTeleport player to point with (#x,#y) client coordinates at ground(water) level in zone #zoneid or current zone if #zoneid not provided. You can look up zone using .lookup area $namepart'),
('gobject add',2,'Syntax: .gobject add #id <spawntimeSecs>\r\n\r\nAdd a game object from game object templates to the world at your current location using the #id.\r\nspawntimesecs sets the spawntime, it is optional.\r\n\r\nNote: this is a copy of .gameobject.'),
('gobject delete',2,'Syntax: .gobject delete #go_guid\r\nDelete gameobject with guid #go_guid.'),
('gobject move',2,'Syntax: .gobject move #goguid [#x #y #z]\r\n\r\nMove gameobject #goguid to character coordinates (or to (#x,#y,#z) coordinates if its provide).'),
('gobject near',2,'Syntax: .gobject near  [#distance]\r\n\r\nOutput gameobjects at distance #distance from player. Output gameobject guids and coordinates sorted by distance from character. If #distance not provided use 10 as default value.'),
('gobject target',2,'Syntax: .gobject target [#go_id|#go_name_part]\r\n\r\nLocate and show position nearest gameobject. If #go_id or #go_name_part provide then locate and show position of nearest gameobject with gameobject template id #go_id or name included #go_name_part as part.'),
('gobject turn',2,'Syntax: .gobject turn #goguid \r\n\r\nSet for gameobject #goguid orientation same as current character orientation.'),
('goname',1,'Syntax: .goname [$charactername]\r\n\r\nTeleport to the given character. Either specify the character name or click on the character\'s portrait, e.g. when you are in a group. Character can be offline.'),
('gps',1,'Syntax: .gps [$name|$shift-link]\r\n\r\nDisplay the position information for a selected character or creature (also if player name $name provided then for named player, or if creature/gameobject shift-link provided then pointed creature/gameobject if it loaded). Position information includes X, Y, Z, and orientation, map Id and zone Id'),
('groupgo',1,'Syntax: .groupgo [$charactername]\r\n\r\nTeleport the given character and his group to you. Teleported only online characters but original selected group member can be offline.'),
('guid',2,'Syntax: .guid\r\n\r\nDisplay the GUID for the selected character.'),
('guild create',2,'Syntax: .guild create [$GuildLeaderName] \"$GuildName\"\r\n\r\nCreate a guild named $GuildName with the player $GuildLeaderName (or selected) as leader.  Guild name must in quotes.'),
('guild delete',2,'Syntax: .guild delete \"$GuildName\"\r\n\r\nDelete guild $GuildName. Guild name must in quotes.'),
('guild invite',2,'Syntax: .guild invite [$CharacterName] \"$GuildName\"\r\n\r\nAdd player $CharacterName (or selected) into a guild $GuildName. Guild name must in quotes.'),
('guild rank',2,'Syntax: .guild rank $CharacterName #Rank\r\n\r\nSet for $CharacterName rank #Rank in a guild.'),
('guild uninvite',2,'Syntax: .guild uninvite [$CharacterName]\r\n\r\nRemove player $CharacterName (or selected) from a guild.'),
('help',0,'Syntax: .help [$command]\r\n\r\nDisplay usage instructions for the given $command. If no $command provided show list available commands.'),
('hidearea',3,'Syntax: .hidearea #areaid\r\n\r\nHide the area of #areaid to the selected character. If no character is selected, hide this area to you.'),
('honor add',2,'Syntax: .honor add $amount\r\n\r\nAdd a certain amount of honor (gained in lifetime) to the selected player.'),
('honor addkill',2,'Syntax: .honor addkill\r\n\r\nAdd the targeted unit as one of your pvp kills today (you only get honor if it\'s a racial leader or a player)'),
('honor show',1,'Syntax: .honor show\r\n\r\nDisplay the honor stats of the selected player'),
('honor update',2,'Syntax: .honor update\r\n\r\nForce honor to be update'),
('instance listbinds',3,'Syntax: .instance listbinds\r\n  Lists the binds of the selected player.'),
('instance savedata',3,'Syntax: .instance savedata\r\n  Save the InstanceData for the current player\'s map to the DB.'),
('instance stats',3,'Syntax: .instance stats\r\n  Shows statistics about instances.'),
('instance unbind',3,'Syntax: .instance unbind all\r\n  All of the selected player\'s binds will be cleared.\r\n.instance unbind #mapid\r\n Only the specified #mapid instance will be cleared.'),
('itemmove',2,'Syntax: .itemmove #sourceslotid #destinationslotid\r\n\r\nMove an item from slots #sourceslotid to #destinationslotid in your inventory\r\n\r\nNot yet implemented'),
('kick',2,'Syntax: .kick [$charactername]\r\n\r\nKick the given character name from the world. If no character name is provided then the selected player (except for yourself) will be kicked.'),
('learn',3,'Syntax: .learn #spell [all]\r\n\r\nSelected character learn a spell of id #spell. If \'all\' provided then all ranks learned.'),
('learn all',3,'Syntax: .learn all\r\n\r\nLearn all big set different spell maybe useful for Administaror.'),
('learn all_crafts',2,'Syntax: .learn crafts\r\n\r\nLearn all professions and recipes.'),
('learn all_default',1,'Syntax: .learn all_default [$playername]\r\n\r\nLearn for selected/$playername player all default spells for his race/class and spells rewarded by completed quests.'),
('learn all_gm',2,'Syntax: .learn all_gm\r\n\r\nLearn all default spells for Game Masters.'),
('learn all_lang',1,'Syntax: .learn all_lang\r\n\r\nLearn all languages'),
('learn all_myclass',3,'Syntax: .learn all_myclass\r\n\r\nLearn all spells and talents available for his class.'),
('learn all_myspells',3,'Syntax: .learn all_myspells\r\n\r\nLearn all spells (except talents and spells with first rank learned as talent) available for his class.'),
('learn all_mytalents',3,'Syntax: .learn all_mytalents\r\n\r\nLearn all talents (and spells with first rank learned as talent) available for his class.'),
('learn all_recipes',2,'Syntax: .learn all_recipes [$profession]\r\rLearns all recipes of specified profession and sets skill level to max.\rExample: .learn all_recipes enchanting'),
('levelup',3,'Syntax: .levelup [$playername] [#numberoflevels]\r\n\r\nIncrease/decrease the level of character with $playername (or the selected if not name provided) by #numberoflevels Or +1 if no #numberoflevels provided). If #numberoflevels is omitted, the level will be increase by 1. If #numberoflevels is 0, the same level will be restarted. If no character is selected and name not provided, increase your level. Command can be used for offline character. All stats and dependent VALUESrecalculated. At level decrease talents can be reset if need. Also at level decrease equipped items with greater level requirement can be lost.'),
('linkgrave',3,'Syntax: .linkgrave #graveyard_id [alliance|horde]\r\n\r\nLink current zone to graveyard for any (or alliance/horde faction ghosts). This let character ghost from zone teleport to graveyard after die if graveyard is nearest from linked to zone and accept ghost of this faction. Add only single graveyard at another map and only if no graveyards linked (or planned linked at same map).'),
('list creature',3,'Syntax: .list creature #creature_id [#max_count]\r\n\r\nOutput creatures with creature id #creature_id found in world. Output creature guids and coordinates sorted by distance from character. Will be output maximum #max_count creatures. If #max_count not provided use 10 as default value.'),
('list item',3,'Syntax: .list item #item_id [#max_count]\r\n\r\nOutput items with item id #item_id found in all character inventories, mails, auctions, and guild banks. Output item guids, item owner guid, owner account and owner name (guild name and guid in case guild bank). Will be output maximum #max_count items. If #max_count not provided use 10 as default value.'),
('list object', 3, 'Syntax: .list object #gameobject_id [#max_count]\r\n.list object #gameobject_id [world|zone|area|map] [#max_count]\r\nOutput gameobjects with gameobject id #gameobject_id found in world. Output gameobject guids and coordinates sorted by distance from character. Will be output maximum #max_count gameobject. If #max_count not provided use 200 as default value.'),
('list talents',3,'Syntax: .list talents\r\n\r\nShow list all really known (as learned spells) talent rank spells for selected player or self.'),
('list areatriggers', 3, 'Syntax: .list areatriggers\n\nShow areatriggers within the same map (if inside an instanceable map) or area (if inside a continent) as the user.'),
('loadscripts',3,'Syntax: .loadscripts $scriptlibraryname\r\n\r\nUnload current and load the script library $scriptlibraryname or reload current if $scriptlibraryname omitted, in case you changed it while the server was running.'),
('lookup account email',2,'Syntax: .lookup account email $emailpart [#limit] \r\n\r\n Searchs accounts, which email including $emailpart with optional parametr #limit of results. If #limit not provided expected 100.'),
('lookup account ip',2,'Syntax: lookup account ip $ippart [#limit] \r\n\r\n Searchs accounts, which last used ip inluding $ippart (textual) with optional parametr #$limit of results. If #limit not provided expected 100.'),
('lookup account name',2,'Syntax: .lookup account name $accountpart [#limit] \r\n\r\n Searchs accounts, which username including $accountpart with optional parametr #limit of results. If #limit not provided expected 100.'),
('lookup area',1,'Syntax: .lookup area $namepart\r\n\r\nLooks up an area by $namepart, and returns all matches with their area ID\'s.'),
('lookup creature',3,'Syntax: .lookup creature $namepart\r\n\r\nLooks up a creature by $namepart, and returns all matches with their creature ID\'s.'),
('lookup event',2,'Syntax: .lookup event $name\r\nAttempts to find the ID of the event with the provided $name.'),
('lookup faction',3,'Syntax: .lookup faction $name\r\nAttempts to find the ID of the faction with the provided $name.'),
('lookup item',3,'Syntax: .lookup item $itemname\r\n\r\nLooks up an item by $itemname, and returns all matches with their Item ID\'s.'),
('lookup itemset',3,'Syntax: .lookup itemset $itemname\r\n\r\nLooks up an item set by $itemname, and returns all matches with their Item set ID\'s.'),
('lookup object',3,'Syntax: .lookup object $objname\r\n\r\nLooks up an gameobject by $objname, and returns all matches with their Gameobject ID\'s.'),
('lookup player account',2,'Syntax: .lookup player account $accountpart [#limit] \r\n\r\n Searchs players, which account username including $accountpart with optional parametr #limit of results. If #limit not provided expected 100.'),
('lookup player email',2,'Syntax: .lookup player email $emailpart [#limit] \r\n\r\n Searchs players, which account email including $emailpart with optional parametr #limit of results. If #limit not provided expected 100.'),
('lookup player ip',2,'Syntax: .lookup player ip $ippart [#limit] \r\n\r\n Searchs players, which account last used ip inluding $ippart (textual) with optional parametr #limit of results. If #limit not provided expected 100.'),
('lookup pool',2,'Syntax: .lookup pool $pooldescpart\r\n\r\nList of pools (anywhere) with substring in description.'),
('lookup quest',3,'Syntax: .lookup quest $namepart\r\n\r\nLooks up a quest by $namepart, and returns all matches with their quest ID\'s.'),
('lookup skill',3,'Syntax: .lookup skill $$namepart\r\n\r\nLooks up a skill by $namepart, and returns all matches with their skill ID\'s.'),
('lookup spell',3,'Syntax: .lookup spell $namepart\r\n\r\nLooks up a spell by $namepart, and returns all matches with their spell ID\'s.'),
('lookup taxinode',3,'Syntax: .lookup taxinode $substring\r\n\r\nSearch and output all taxinodes with provide $substring in name.'),
('lookup tele',1,'Syntax: .lookup tele $substring\r\n\r\nSearch and output all .tele command locations with provide $substring in name.'),
('maxskill',3,'Syntax: .maxskill\r\nSets all skills of the targeted player to their maximum VALUESfor its current level.'),
('modify aspeed',1,'Syntax: .modify aspeed #rate\r\n\r\nModify all speeds -run,swim,run back,swim back- of the selected player to \"normalbase speed for this move type\"*rate. If no player is selected, modify your speed.\r\n\r\n #rate may range from 0.1 to 10.'),
('modify bwalk',1,'Syntax: .modify bwalk #rate\r\n\r\nModify the speed of the selected player while running backwards to \"normal walk back speed\"*rate. If no player is selected, modify your speed.\r\n\r\n #rate may range from 0.1 to 10.'),
('modify drunk',1,'Syntax: .modify drunk #value\r\n Set drunk level to #value (0..100). Value 0 remove drunk state, 100 is max drunked state.'),
('modify energy',1,'Syntax: .modify energy #energy\r\n\r\nModify the energy of the selected player. If no player is selected, modify your energy.'),
('modify faction',1,'Syntax: .modify faction #factionid #flagid #npcflagid #dynamicflagid\r\n\r\nModify the faction and flags of the selected creature. Without arguments, display the faction and flags of the selected creature.'),
('modify fly',1,'Syntax: .modify fly #rate\r\n.fly #rate\r\n\r\nModify the flying speed of the selected player to \"normal base fly speed\"*rate. If no player is selected, modify your fly.\r\n\r\n #rate may range from 0.1 to 10.'),
('modify gender',2,'Syntax: .modify gender male/female\r\n\r\nChange gender of selected player.'),
('modify honor',1,'Syntax: .modify honor $field $value\r\n\r\nAdd $value to an honor $field (it\'s just for debug , values won\'t be saved at player logout)\r\nField list: points / rank / todaykills / yesterdaykills / yesterdayhonor /\r\nthisweekkills / thisweekhonor / lastweekkills / lastweekhonor /\r\nlastweekstanding / lifetimedishonorablekills / lifetimehonorablekills '),
('modify hp',1,'Syntax: .modify hp #newhp\r\n\r\nModify the hp of the selected player. If no player is selected, modify your hp.'),
('modify mana',1,'Syntax: .modify mana #newmana\r\n\r\nModify the mana of the selected player. If no player is selected, modify your mana.'),
('modify money',1,'Syntax: .modify money #money\r\n.money #money\r\n\r\nAdd or remove money to the selected player. If no player is selected, modify your money.\r\n\r\n #gold can be negative to remove money.'),
('modify morph',2,'Syntax: .modify morph #displayid\r\n\r\nChange your current model id to #displayid.'),
('modify mount',1,'Syntax: .modify mount [fast|slow]\r\n\r\nProvide selected player a random unusual land mount.'),
('modify rage',1,'Syntax: .modify rage #newrage\r\n\r\nModify the rage of the selected player. If no player is selected, modify your rage.'),
('modify rep',2,'Syntax: .modify rep #repId (#repvalue | $rankname [#delta])\r\nSets the selected players reputation with faction #repId to #repvalue or to $reprank.\r\nIf the reputation rank name is provided, the resulting reputation will be the lowest reputation for that rank plus the delta amount, if specified.\r\nYou can use \'.pinfo rep\' to list all known reputation ids, or use \'.lookup faction $name\' to locate a specific faction id.'),
('modify scale',1,'Syntax: .modify scale #scale\r\n\r\nChange model scale for targeted player (util relogin) or creature (until respawn).'),
('modify speed',1,'Syntax: .modify speed #rate\r\n.speed #rate\r\n\r\nModify the running speed of the selected player to \"normal base run speed\"*rate. If no player is selected, modify your speed.\r\n\r\n #rate may range from 0.1 to 10.'),
('modify standstate',2,'Syntax: .modify standstate #emoteid\r\n\r\nChange the emote of your character while standing to #emoteid.'),
('modify swim',1,'Syntax: .modify swim #rate\r\n\r\nModify the swim speed of the selected player to \"normal swim speed\"*rate. If no player is selected, modify your speed.\r\n\r\n #rate may range from 0.1 to 10.'),
('movement movegens',2,'Syntax: .movement movegens  Show movement generators stack for selected creature or player.'),
('movement cometome',3,'Syntax: .movement cometome  Move selected creature to you.'),
('movement movespeed',2,'Syntax: .movement movespeed  Show speed of selected creature.'),
('mute',1,'Syntax: .mute [$playerName] $timeInMinutes\r\n\r\nDisible chat messaging for any character from account of character $playerName (or currently selected) at $timeInMinutes minutes. Player can be offline.'),
('namego',1,'Syntax: .namego [$charactername]\r\n\r\nTeleport the given character to you. Character can be offline.'),
('neargrave',3,'Syntax: .neargrave [alliance|horde]\r\n\r\nFind nearest graveyard linked to zone (or only nearest from accepts alliance or horde faction ghosts).'),
('notify',1,'Syntax: .notify $MessageToBroadcast\r\n\r\nSend a global message to all players online in screen.'),
('npc add',2,'Syntax: .npc add #creatureid\r\n\r\nSpawn a creature by the given template id of #creatureid.'),
('npc additem',2,'Syntax: .npc additem #itemId <#maxcount><#incrtime>r\r\n\r\nAdd item #itemid to item list of selected vendor. Also optionally set max count item in vendor item list and time to item count restoring.'),
('npc addweapon',3,'Not yet implemented.'),
('npc aiinfo',2,'Syntax: .npc npc aiinfo\r\n\r\nShow npc AI and script information.'),
('npc allowmove',3,'Syntax: .npc allowmove\r\n\r\nEnable or disable movement creatures in world. Not implemented.'),
('npc changelevel',2,'Syntax: .npc changelevel #level\r\n\r\nChange the level of the selected creature to #level.\r\n\r\n#level may range from 1 to 63.'),
('npc delete',2,'Syntax: .npc delete [#guid]\r\n\r\nDelete creature with guid #guid (or the selected if no guid is provided)'),
('npc delitem',2,'Syntax: .npc delitem #itemId\r\n\r\nRemove item #itemid from item list of selected vendor.'),
('npc factionid',2,'Syntax: .npc factionid #factionid\r\n\r\nSet the faction of the selected creature to #factionid.'),
('npc flag',2,'Syntax: .npc flag #npcflag\r\n\r\nSet the NPC flags of creature template of the selected creature and selected creature to #npcflag. NPC flags will applied to all creatures of selected creature template after server restart or grid unload/load.'),
('npc follow',2,'Syntax: .npc follow\r\n\r\nSelected creature start follow you until death/fight/etc.'),
('npc info',3,'Syntax: .npc info\r\n\r\nDisplay a list of details for the selected creature.\r\n\r\nThe list includes:\r\n- GUID, Faction, NPC flags, Entry ID, Model ID,\r\n- Level,\r\n- Health (current/maximum),\r\n\r\n- Field flags, dynamic flags, faction template, \r\n- Position information,\r\n- and the creature type, e.g. if the creature is a vendor.'),
('npc move',2,'Syntax: .npc move [#creature_guid]\r\n\r\nMove the targeted creature spawn point to your coordinates.'),
('npc name',2,'Syntax: .npc name $name\r\n\r\nChange the name of the selected creature or character to $name.\r\n\r\nCommand disabled.'),
('npc playemote',3,'Syntax: .npc playemote #emoteid\r\n\r\nMake the selected creature emote with an emote of id #emoteid.'),
('npc say',1,'Syntax: .npc say #text\r\nMake the selected npc says #text.'),
('npc setdeathstate',2,'Syntax: .npc setdeathstate on/off\r\n\r\nSet default death state (dead/alive) for npc at spawn.'),
('npc setmodel',2,'Syntax: .npc setmodel #displayid\r\n\r\nChange the model id of the selected creature to #displayid.'),
('npc setmovetype',2,'Syntax: .npc setmovetype [#creature_guid] stay/random/way [NODEL]\r\n\r\nSet for creature pointed by #creature_guid (or selected if #creature_guid not provided) movement type and move it to respawn position (if creature alive). Any existing waypoints for creature will be removed from the database if you do not use NODEL. If the creature is dead then movement type will applied at creature respawn.\r\nMake sure you use NODEL, if you want to keep the waypoints.'),
('npc spawndist',2,'Syntax: .npc spawndist #dist\r\n\r\nAdjust spawndistance of selected creature to dist.'),
('npc spawntime',2,'Syntax: .npc spawntime #time \r\n\r\nAdjust spawntime of selected creature to time.'),
('npc subname',2,'Syntax: .npc subname $Name\r\n\r\nChange the subname of the selected creature or player to $Name.\r\n\r\nCommand disabled.'),
('npc tame',2,'Syntax: .npc tame\r\n\r\nTame selected creature (tameable non pet creature). You don\'t must have pet.'),
('npc textemote',1,'Syntax: .npc textemote #emoteid\r\n\r\nMake the selected creature to do textemote with an emote of id #emoteid.'),
('npc unfollow',2,'Syntax: .npc unfollow\r\n\r\nSelected creature (non pet) stop follow you.'),
('npc whisper',1,'Syntax: .npc whisper #playerguid #text\r\nMake the selected npc whisper #text to  #playerguid.'),
('npc yell',1,'Syntax: .npc yell #text\r\nMake the selected npc yells #text.'),
('pdump load',3,'Syntax: .pdump load $filename $account [$newname] [$newguid]\r\nLoad character dump from dump file into character list of $account with saved or $newname, with saved (or first free) or $newguid guid.'),
('pdump write',3,'Syntax: .pdump write $filename $playerNameOrGUID\r\nWrite character dump with name/guid $playerNameOrGUID to file $filename.'),
('pinfo',2,'Syntax: .pinfo [$player_name]\r\n\r\nOutput account information for selected player or player find by $player_name.'),
('pool',2,'Syntax: .pool #pool_id\r\n\r\nPool information and full list creatures/gameobjects included in pool.'),
('pool list',2,'Syntax: .pool list\r\n\r\nList of pools with spawn in current map (only work in instances. Non-instanceable maps share pool system state os useless attempt get all pols at all continents.'),
('pool spawns',2,'Syntax: .pool spawns #pool_id\r\n\r\nList current creatures/objects listed in pools (or in specific #pool_id) and spawned (added to grid data, not meaning show in world.'),
('quest add',3,'Syntax: .quest add #quest_id\r\n\r\nAdd to character quest log quest #quest_id. Quest started from item can\'t be added by this command but correct .additem call provided in command output.'),
('quest complete',3,'Syntax: .quest complete #questid\r\nMark all quest objectives as completed for target character active quest. After this target character can go and get quest reward.'),
('quest remove',3,'Syntax: .quest remove #quest_id\r\n\r\nSet quest #quest_id state to not completed and not active (and remove from active quest list) for selected player.'),
('quit',4,'Syntax: quit\r\n\r\nClose RA connection. Command must be typed fully (quit).'),
('recall',1,'Syntax: .recall [$playername]\r\n\r\nTeleport $playername or selected player to the place where he has been before last use of a teleportation command. If no $playername is entered and no player is selected, it will teleport you.'),
('reload all',3,'Syntax: .reload all\r\n\r\nReload all tables with reload support added and that can be _safe_ reloaded.'),
('reload all_area',3,'Syntax: .reload all_area\r\n\r\nReload all `areatrigger_*` tables if reload support added for this table and this table can be _safe_ reloaded.'),
('reload all_eventai',3,'Syntax: .reload all_eventai\r\n\r\nReload `creature_ai_*` tables if reload support added for these tables and these tables can be _safe_ reloaded.'),
('reload all_item',3,'Syntax: .reload all_item\r\n\r\nReload `item_required_target`, `page_texts` and `item_enchantment_template` tables.'),
('reload all_locales',3,'Syntax: .reload all_locales\r\n\r\nReload all `locales_*` tables with reload support added and that can be _safe_ reloaded.'),
('reload all_loot',3,'Syntax: .reload all_loot\r\n\r\nReload all `*_loot_template` tables. This can be slow operation with lags for server run.'),
('reload all_npc',3,'Syntax: .reload all_npc\r\n\r\nReload `points_of_interest` and `npc_*` tables if reload support added for these tables and these tables can be _safe_ reloaded.'),
('reload all_quest',3,'Syntax: .reload all_quest\r\n\r\nReload all quest related tables if reload support added for this table and this table can be _safe_ reloaded.'),
('reload all_scripts',3,'Syntax: .reload all_scripts\r\n\r\nReload `dbscripts_on_*` tables.'),
('reload all_spell',3,'Syntax: .reload all_spell\r\n\r\nReload all `spell_*` tables with reload support added and that can be _safe_ reloaded.'),
('reload config',3,'Syntax: .reload config\r\n\r\nReload config settings (by default stored in mangosd.conf). Not all settings can be change at reload: some new setting values will be ignored until restart, some values will applied with delay or only to new objects/maps, some values will explicitly rejected to change at reload.'),
('repairitems',2,'Syntax: .repairitems\r\n\r\nRepair all selected player\'s items.'),
('reset all',3,'Syntax: .reset all spells\r\n\r\nSyntax: .reset all talents\r\n\r\nRequest reset spells or talents at next login each existed character.'),
('reset honor',3,'Syntax: .reset honor [Playername]\r\n  Reset all honor data for targeted character.'),
('reset level',3,'Syntax: .reset level [Playername]\r\n  Reset level to 1 including reset stats and talents.  Equipped items with greater level requirement can be lost.'),
('reset spells',3,'Syntax: .reset spells [Playername]\r\n  Removes all non-original spells from spellbook.\r\n. Playername can be name of offline character.'),
('reset stats',3,'Syntax: .reset stats [Playername]\r\n  Resets(recalculate) all stats of the targeted player to their original VALUESat current level.'),
('reset talents',3,'Syntax: .reset talents [Playername]\r\n  Removes all talents of the targeted player. Playername can be name of offline character.'),
('respawn',3,'Syntax: .respawn\r\n\r\nRespawn selected creature or respawn all nearest creatures (if none selected) and GO without waiting respawn time expiration.'),
('revive',3,'Syntax: .revive\r\n\r\nRevive the selected player. If no player is selected, it will revive you.'),
('save',0,'Syntax: .save\r\n\r\nSaves your character.'),
('saveall',1,'Syntax: .saveall\r\n\r\nSave all characters in game.'),
('send items',3,'Syntax: .send items #playername \"#subject\" \"#text\" itemid1[:count1] itemid2[:count2] ... itemidN[:countN]\r\n\r\nSend a mail to a player. Subject and mail text must be in \"\". If for itemid not provided related count values then expected 1, if count > max items in stack then items will be send in required amount stacks. All stacks amount in mail limited to 12.'),
('send mail',1,'Syntax: .send mail #playername \"#subject\" \"#text\"\r\n\r\nSend a mail to a player. Subject and mail text must be in \"\".'),
('send mass items',3,'Syntax: .send mass items #racemask|$racename|alliance|horde|all \"#subject\" \"#text\" itemid1[:count1] itemid2[:count2] ... itemidN[:countN]\r\n\r\nSend a mail to players. Subject and mail text must be in \"\". If for itemid not provided related count values then expected 1, if count > max items in stack then items will be send in required amount stacks. All stacks amount in mail limited to 12.'),
('send mass mail',3,'Syntax: .send mass mail #racemask|$racename|alliance|horde|all \"#subject\" \"#text\"\r\n\r\nSend a mail to players. Subject and mail text must be in \"\".'),
('send mass money',3,'Syntax: .send mass money #racemask|$racename|alliance|horde|all \"#subject\" \"#text\" #money\r\n\r\nSend mail with money to players. Subject and mail text must be in \"\".'),
('send message',3,'Syntax: .send message $playername $message\r\n\r\nSend screen message to player from ADMINISTRATOR.'),
('send money',3,'Syntax: .send money #playername \"#subject\" \"#text\" #money\r\n\r\nSend mail with money to a player. Subject and mail text must be in \"\".'),
('server corpses',2,'Syntax: .server corpses\r\n\r\nTriggering corpses expire check in world.'),
('server exit',4,'Syntax: .server exit\r\n\r\nTerminate mangosd NOW. Exit code 0.'),
('server idlerestart',3,'Syntax: .server idlerestart #delay\r\n\r\nRestart the server after #delay seconds if no active connections are present (no players). Use #exist_code or 2 as program exist code.'),
('server idlerestart cancel',3,'Syntax: .server idlerestart cancel\r\n\r\nCancel the restart/shutdown timer if any.'),
('server idleshutdown',3,'Syntax: .server idleshutdown #delay [#exist_code]\r\n\r\nShut the server down after #delay seconds if no active connections are present (no players). Use #exist_code or 0 as program exist code.'),
('server idleshutdown cancel',3,'Syntax: .server idleshutdown cancel\r\n\r\nCancel the restart/shutdown timer if any.'),
('server info',0,'Syntax: .server info\r\n\r\nDisplay server version and the number of connected players.'),
('server log filter',4,'Syntax: .server log filter [($filtername|all) (on|off)]\r\n\r\nShow or set server log filters. If used \"all\" then all filters will be set to on/off state.'),
('server log level',4,'Syntax: .server log level [#level]\r\n\r\nShow or set server log level (0 - errors only, 1 - basic, 2 - detail, 3 - debug).'),
('server motd',0,'Syntax: .server motd\r\n\r\nShow server Message of the day.'),
('server plimit',3,'Syntax: .server plimit [#num|-1|-2|-3|reset|player|moderator|gamemaster|administrator]\r\n\r\nWithout arg show current player amount and security level limitations for login to server, with arg set player linit ($num > 0) or securiti limitation ($num < 0 or security leme name. With `reset` sets player limit to the one in the config file'),
('server restart',3,'Syntax: .server restart #delay\r\n\r\nRestart the server after #delay seconds. Use #exist_code or 2 as program exist code.'),
('server restart cancel',3,'Syntax: .server restart cancel\r\n\r\nCancel the restart/shutdown timer if any.'),
('server set motd',3,'Syntax: .server set motd $MOTD\r\n\r\nSet server Message of the day.'),
('server shutdown',3,'Syntax: .server shutdown #delay [#exit_code]\r\n\r\nShut the server down after #delay seconds. Use #exit_code or 0 as program exit code.'),
('server shutdown cancel',3,'Syntax: .server shutdown cancel\r\n\r\nCancel the restart/shutdown timer if any.'),
('setskill',3,'Syntax: .setskill #skill #level [#max]\r\n\r\nSet a skill of id #skill with a current skill value of #level and a maximum value of #max (or equal current maximum if not provide) for the selected character. If no character is selected, you learn the skill.'),
('showarea',3,'Syntax: .showarea #areaid\r\n\r\nReveal the area of #areaid to the selected character. If no character is selected, reveal this area to you.'),
('stable',3,'Syntax: .stable\r\n\r\nShow your pet stable.'),
('start',0,'Syntax: .start\r\n\r\nTeleport you to the starting area of your character.'),
('taxicheat',1,'Syntax: .taxicheat on/off\r\n\r\nTemporary grant access or remove to all taxi routes for the selected character. If no character is selected, hide or reveal all routes to you.\r\n\r\nVisited taxi nodes sill accessible after removing access.'),
('tele',1,'Syntax: .tele #location\r\n\r\nTeleport player to a given location.'),
('tele add',3,'Syntax: .tele add $name\r\n\r\nAdd current your position to .tele command target locations list with name $name.'),
('tele del',3,'Syntax: .tele del $name\r\n\r\nRemove location with name $name for .tele command locations list.'),
('tele group',1,'Syntax: .tele group#location\r\n\r\nTeleport a selected player and his group members to a given location.'),
('tele name',1,'Syntax: .tele name [#playername] #location\r\n\r\nTeleport the given character to a given location. Character can be offline.'),
('ticket',2,'Syntax: .ticket #id\r\n\r\nActs as an alias of: ".ticket read"\r\n'),
('ticket discard',2,'Syntax: .ticket discard #id [$conclusion]\r\n\r\nClose GM ticket with number #id as discarded. If $conclusion is provided, it will be visible to the author as well.'),
('ticket escalate',2,'Syntax: .ticket escalate #id\r\n\r\nAttempt to escalate GM ticket with number #id. Current assignee will be unassigned on success.'),
('ticket go',2,'Syntax: .ticket go #id\r\n\r\nAttempt to teleport to the location where GM ticket with number #id was originally created.'),
('ticket goname',2,'Syntax: .ticket goname #id\r\n\r\nAttempt to teleport to the author of the GM ticket with number #id.'),
('ticket note',2,'Syntax: .ticket note #id $message\r\n\r\nAdd a note visible only to GMs to the GM ticket with number #id.'),
('ticket read',2,'Syntax: .ticket read #id\r\n\r\nShow contents of GM ticket with number #id.'),
('ticket resolve',2,'Syntax: .ticket resolve #id [$conclusion]\r\n\r\nClose GM ticket with number #id as resolved. If $conclusion is provided, it will be visible to player as well.'),
('ticket sort',2,'Syntax: .ticket sort #id #categoryid\r\n\r\nAttempt to assign the GM ticket with number #id with a category by id #categoryid.'),
('ticket whisper',2,'Syntax: .ticket whisper #id $message\r\n\r\nAttempt to answer in-game GM ticket with number #id by sending whisper with $message. Ticket will be assigned regardless of author\'s online status.'),
('tickets',2,'Syntax: .tickets [on|off|[#categoryid #max|#max] [online]]\r\n\r\nIf "on"/"off" provided, enable or disable in-game GM ticket queue notifications and GM ticket alerts. Acts as an alias of ".tickets list" otherwise.\r\n'),
('tickets queue',3,'Syntax: .tickets queue on|off\r\n\r\nEnable or disable GM tickets queue until next restart or administrator\'s command.'),
('tickets list',2,'Syntax: .tickets list [#categoryid #max|#max] [online]\r\n\r\nShow current GM tickets queue. If #categoryid is provided, show only GM tickets from that category.'),
('trigger',2,'Syntax: .trigger [#trigger_id|$trigger_shift-link|$trigger_target_shift-link]\r\n\r\nShow detail infor about areatrigger with id #trigger_id or trigger id associated with shift-link. If areatrigger id or shift-link not provided then selected nearest areatrigger at current map.'),
('trigger active',2,'Syntax: .trigger active\r\n\r\nShow list of areatriggers with activation zone including current character position.'),
('trigger near',2,'Syntax: .trigger near [#distance]\r\n\r\nOutput areatriggers at distance #distance from player. If #distance not provided use 10 as default value.'),
('unaura',3,'Syntax: .unaura #spellid\r\n\r\nRemove aura due to spell #spellid from the selected Unit.'),
('unban account',3,'Syntax: .unban account $Name\r\nUnban accounts for account name pattern.'),
('unban character',3,'Syntax: .unban character $Name\r\nUnban accounts for character name pattern.'),
('unban ip',3,'Syntax : .unban ip $Ip\r\nUnban accounts for IP pattern.'),
('unlearn',3,'Syntax: .unlearn #spell [all]\r\n\r\nUnlearn for selected player a spell #spell.  If \'all\' provided then all ranks unlearned.'),
('unmute',1,'Syntax: .unmute $playerName\r\n\r\nRestore chat messaging for any character from account of character $playerName.'),
('waterwalk',2,'Syntax: .waterwalk on/off\r\n\r\nSet on/off waterwalk state for selected player.'),
('wchange',3,'Syntax: .wchange #weathertype #status\r\n\r\nSet current weather to #weathertype with an intensity of #status.\r\n\r\n#weathertype can be 1 for rain, 2 for snow, and 3 for sand. #status can be 0 for disabled, and 1 for enabled.'),
('whispers',1,'Syntax: .whispers on|off\r\nEnable/disable accepting whispers by GM from players. By default use mangosd.conf setting.'),
('wp add',2,'Syntax: .wp add [Selected Creature or dbGuid] [pathId [wpOrigin] ]'),
('wp export',3,'Syntax: .wp export [#creature_guid or Select a Creature] $filename'),
('wp modify',2,'Syntax: .wp modify command [dbGuid, id] [value]\r\nwhere command must be one of: waittime  | scriptid | orientation | del | move\r\nIf no waypoint was selected, one can be chosen with dbGuid and id.\r\nThe commands have the following meaning:\r\n waittime (Set the time the npc will wait at a point (in ms))\r\n scriptid (Set the DB-Script that will be executed when the wp is reached)\r\n orientation (Set the orientation of this point) \r\n del (Remove the waypoint from the path)\r\n move (Move the wayoint to the current position of the player)'),
('wp show',2,'Syntax: .wp show command [dbGuid] [pathId [wpOrigin] ]\r\nwhere command can have one of the following values\r\non (to show all related wp)\r\nfirst (to see only first one)\r\nlast (to see only last one)\r\noff (to hide all related wp)\r\ninfo (to get more info about theses wp)\r\n\r\nFor using info you have to do first show on and than select a Visual-Waypoint and do the show info!\r\nwith pathId and wpOrigin you can specify which path to show (optional)'),
('wr', 0, 'Syntax: .wr [on/off]\r\n\r\nEnable or disable whisper restriction. Whisper restriction prevents players from whispering you that are not in your friends list, or a member of your guild or party.');
/*!40000 ALTER TABLE `command` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `conditions`
--

DROP TABLE IF EXISTS `conditions`;
CREATE TABLE `conditions` (
  `condition_entry` mediumint(8) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `type` tinyint(3) NOT NULL DEFAULT '0' COMMENT 'Type of the condition',
  `value1` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'data field one for the condition',
  `value2` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'data field two for the condition',
  `value3` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'data field three for the condition',
  `value4` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'data field four for the condition',
  `flags` TINYINT UNSIGNED NOT NULL DEFAULT '0',
  `comments` VARCHAR(500) DEFAULT '',
  PRIMARY KEY (`condition_entry`),
  UNIQUE KEY `unique_conditions` (`type`,`value1`,`value2`,`value3`,`value4`,`flags`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Condition System';

--
-- Dumping data for table `conditions`
--

LOCK TABLES `conditions` WRITE;
/*!40000 ALTER TABLE `conditions` DISABLE KEYS */;
/*!40000 ALTER TABLE `conditions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature`
--

DROP TABLE IF EXISTS `creature`;
CREATE TABLE `creature` (
  `guid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Global Unique Identifier',
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Creature Identifier',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `spawnMask` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `position_x` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `position_y` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `position_z` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `orientation` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `spawntimesecsmin` int(10) unsigned NOT NULL DEFAULT '120' COMMENT 'Creature respawn time minimum',
  `spawntimesecsmax` int(10) unsigned NOT NULL DEFAULT '120' COMMENT 'Creature respawn time maximum',
  `spawndist` float NOT NULL DEFAULT '5',
  `MovementType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  KEY `idx_map` (`map`),
  KEY `index_id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature`
--

LOCK TABLES `creature` WRITE;
/*!40000 ALTER TABLE `creature` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_addon`
--

DROP TABLE IF EXISTS `creature_addon`;
CREATE TABLE `creature_addon` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `mount` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `stand_state` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `sheath_state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `emote` int(10) unsigned NOT NULL DEFAULT '0',
  `moveflags` int(10) unsigned NOT NULL DEFAULT '0',
  `auras` text,
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creature_addon`
--

LOCK TABLES `creature_addon` WRITE;
/*!40000 ALTER TABLE `creature_addon` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_addon` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_ai_scripts`
--

DROP TABLE IF EXISTS `creature_ai_scripts`;
CREATE TABLE `creature_ai_scripts` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `creature_id` int(11) NOT NULL DEFAULT '0' COMMENT 'Creature Template Identifier',
  `event_type` tinyint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Event Type',
  `event_inverse_phase_mask` int(11) NOT NULL DEFAULT '0' COMMENT 'Mask which phases this event will not trigger in',
  `event_chance` int(3) unsigned NOT NULL DEFAULT '100',
  `event_flags` int(3) unsigned NOT NULL DEFAULT '0',
  `event_param1` int(11) NOT NULL DEFAULT '0',
  `event_param2` int(11) NOT NULL DEFAULT '0',
  `event_param3` int(11) NOT NULL DEFAULT '0',
  `event_param4` int(11) NOT NULL DEFAULT '0',
  `event_param5` int(11) NOT NULL DEFAULT '0',
  `event_param6` int(11) NOT NULL DEFAULT '0',
  `action1_type` tinyint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Action Type',
  `action1_param1` int(11) NOT NULL DEFAULT '0',
  `action1_param2` int(11) NOT NULL DEFAULT '0',
  `action1_param3` int(11) NOT NULL DEFAULT '0',
  `action2_type` tinyint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Action Type',
  `action2_param1` int(11) NOT NULL DEFAULT '0',
  `action2_param2` int(11) NOT NULL DEFAULT '0',
  `action2_param3` int(11) NOT NULL DEFAULT '0',
  `action3_type` tinyint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Action Type',
  `action3_param1` int(11) NOT NULL DEFAULT '0',
  `action3_param2` int(11) NOT NULL DEFAULT '0',
  `action3_param3` int(11) NOT NULL DEFAULT '0',
  `comment` varchar(255) NOT NULL DEFAULT '' COMMENT 'Event Comment',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='EventAI Scripts';

--
-- Dumping data for table `creature_ai_scripts`
--

LOCK TABLES `creature_ai_scripts` WRITE;
/*!40000 ALTER TABLE `creature_ai_scripts` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_ai_scripts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_ai_summons`
--

DROP TABLE IF EXISTS `creature_ai_summons`;
CREATE TABLE `creature_ai_summons` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Location Identifier',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `spawntimesecs` int(11) unsigned NOT NULL DEFAULT '120',
  `comment` varchar(255) NOT NULL DEFAULT '' COMMENT 'Summon Comment',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='EventAI Summoning Locations';

--
-- Dumping data for table `creature_ai_summons`
--

LOCK TABLES `creature_ai_summons` WRITE;
/*!40000 ALTER TABLE `creature_ai_summons` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_ai_summons` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_battleground`
--

DROP TABLE IF EXISTS `creature_battleground`;
CREATE TABLE `creature_battleground` (
  `guid` int(10) unsigned NOT NULL COMMENT 'Creature''s GUID',
  `event1` tinyint(3) unsigned NOT NULL COMMENT 'main event',
  `event2` tinyint(3) unsigned NOT NULL COMMENT 'sub event',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature battleground indexing system';

--
-- Dumping data for table `creature_battleground`
--

LOCK TABLES `creature_battleground` WRITE;
/*!40000 ALTER TABLE `creature_battleground` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_battleground` ENABLE KEYS */;
UNLOCK TABLES;

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

--
-- Table structure for table `creature_spawn_data`
--

DROP TABLE IF EXISTS `creature_spawn_data`;
CREATE TABLE `creature_spawn_data` (
  `Guid` int unsigned NOT NULL COMMENT 'guid of creature',
  `Id` int unsigned NOT NULL COMMENT 'ID of template',
  PRIMARY KEY (`Guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Spawn Data)';

--
-- Dumping data for table `creature_spawn_data`
--

LOCK TABLES `creature_spawn_data` WRITE;
/*!40000 ALTER TABLE `creature_spawn_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_spawn_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_spawn_data_template`
--

DROP TABLE IF EXISTS `creature_spawn_data_template`;
CREATE TABLE `creature_spawn_data_template` (
  `Entry` int unsigned NOT NULL COMMENT 'ID of template',
  `NpcFlags` int NOT NULL DEFAULT '-1',
  `UnitFlags` bigint NOT NULL DEFAULT '-1',
  `Faction` int unsigned NOT NULL DEFAULT '0',
  `ModelId` mediumint unsigned NOT NULL DEFAULT '0',
  `EquipmentId` mediumint NOT NULL DEFAULT '-1',
  `CurHealth` int unsigned NOT NULL DEFAULT '0',
  `CurMana` int unsigned NOT NULL DEFAULT '0',
  `SpawnFlags` int unsigned NOT NULL DEFAULT '0',
  `RelayId` int unsigned NOT NULL DEFAULT '0',
  `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `Name` VARCHAR(200) NOT NULL,
  PRIMARY KEY (`Entry`,`UnitFlags`,`ModelId`,`EquipmentId`,`CurHealth`,`CurMana`,`SpawnFlags`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Spawn Data Template)';

--
-- Dumping data for table `creature_spawn_data_template`
--

LOCK TABLES `creature_spawn_data_template` WRITE;
/*!40000 ALTER TABLE `creature_spawn_data_template` DISABLE KEYS */;
INSERT INTO `creature_spawn_data_template` (`Entry`) VALUES (0);
/*!40000 ALTER TABLE `creature_spawn_data_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_spawn_entry`
--

DROP TABLE IF EXISTS `creature_spawn_entry`;
CREATE TABLE `creature_spawn_entry` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Spawn Entry)';

--
-- Dumping data for table `creature_spawn_entry`
--

LOCK TABLES `creature_spawn_entry` WRITE;
/*!40000 ALTER TABLE `creature_spawn_entry` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_spawn_entry` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_equip_template`
--

DROP TABLE IF EXISTS `creature_equip_template`;
CREATE TABLE `creature_equip_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Unique entry',
  `equipentry1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `equipentry2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `equipentry3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Creature System (Equipment)';

--
-- Dumping data for table `creature_equip_template`
--

LOCK TABLES `creature_equip_template` WRITE;
/*!40000 ALTER TABLE `creature_equip_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_equip_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_involvedrelation`
--

DROP TABLE IF EXISTS `creature_involvedrelation`;
CREATE TABLE `creature_involvedrelation` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature_involvedrelation`
--

LOCK TABLES `creature_involvedrelation` WRITE;
/*!40000 ALTER TABLE `creature_involvedrelation` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_involvedrelation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_linking`
--

DROP TABLE IF EXISTS `creature_linking`;
CREATE TABLE `creature_linking` (
  `guid` int(10) unsigned NOT NULL COMMENT 'creature.guid of the slave mob that is linked',
  `master_guid` int(10) unsigned NOT NULL COMMENT 'master to trigger events',
  `flag` mediumint(8) unsigned NOT NULL COMMENT 'flag - describing what should happen when',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature Linking System';

--
-- Dumping data for table `creature_linking`
--

LOCK TABLES `creature_linking` WRITE;
/*!40000 ALTER TABLE `creature_linking` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_linking` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_linking_template`
--

DROP TABLE IF EXISTS `creature_linking_template`;
CREATE TABLE `creature_linking_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'creature_template.entry of the slave mob that is linked',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Id of map of the mobs',
  `master_entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'master to trigger events',
  `flag` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'flag - describing what should happen when',
  `search_range` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'search_range - describing in which range (spawn-coords) master and slave are linked together',
  PRIMARY KEY (`entry`,`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature Linking System';

--
-- Dumping data for table `creature_linking_template`
--

LOCK TABLES `creature_linking_template` WRITE;
/*!40000 ALTER TABLE `creature_linking_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_linking_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_loot_template`
--

DROP TABLE IF EXISTS `creature_loot_template`;
CREATE TABLE `creature_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'entry 0 used for player insignia loot',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `creature_loot_template`
--

LOCK TABLES `creature_loot_template` WRITE;
/*!40000 ALTER TABLE `creature_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_model_info`
--

DROP TABLE IF EXISTS `creature_model_info`;
CREATE TABLE `creature_model_info` (
  `modelid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `bounding_radius` float NOT NULL DEFAULT '0',
  `combat_reach` float NOT NULL DEFAULT '0',
  `SpeedWalk` FLOAT NOT NULL DEFAULT '1' COMMENT 'Default walking speed for any creature with model',
  `SpeedRun` FLOAT NOT NULL DEFAULT '1.14286' COMMENT 'Default running speed for any creature with model',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '2',
  `modelid_other_gender` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `modelid_alternative` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`modelid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Creature System (Model related info)';

--
-- Dumping data for table `creature_model_info`
--

LOCK TABLES `creature_model_info` WRITE;
/*!40000 ALTER TABLE `creature_model_info` DISABLE KEYS */;
INSERT INTO `creature_model_info` VALUES
(49,0.306,1.5,1,1.14286,0,50,0),
(50,0.208,1.5,1,1.14286,1,49,0),
(51,0.372,1.5,1,1.14286,0,52,0),
(52,0.236,1.5,1,1.14286,1,51,0),
(53,0.347,1.5,1,1.14286,0,54,0),
(54,0.347,1.5,1,1.14286,1,53,0),
(55,0.389,1.5,1,1.14286,0,56,0),
(56,0.306,1.5,1,1.14286,1,55,0),
(57,0.383,1.5,1,1.14286,0,58,0),
(58,0.383,1.5,1,1.14286,1,57,0),
(59,0.9747,1.5,1,1.14286,0,60,0),
(60,0.8725,1.5,1,1.14286,1,59,0),
(1478,0.306,1.5,1,1.14286,0,1479,0),
(1479,0.306,1.5,1,1.14286,1,1478,0),
(1563,0.3519,1.5,1,1.14286,0,1564,0),
(1564,0.3519,1.5,1,1.14286,1,1563,0),
(10045,1,1.5,1,1.14286,2,0,0);
/*!40000 ALTER TABLE `creature_model_info` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_movement`
--

DROP TABLE IF EXISTS `creature_movement`;
CREATE TABLE `creature_movement` (
  `Id` int(10) unsigned NOT NULL COMMENT 'Creature GUID',
  `Point` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `PositionX` float NOT NULL DEFAULT '0',
  `PositionY` float NOT NULL DEFAULT '0',
  `PositionZ` float NOT NULL DEFAULT '0',
  `Orientation` float NOT NULL DEFAULT '0',
  `WaitTime` int(10) unsigned NOT NULL DEFAULT '0',
  `ScriptId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Comment` text,
  PRIMARY KEY (`Id`,`Point`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature_movement`
--

LOCK TABLES `creature_movement` WRITE;
/*!40000 ALTER TABLE `creature_movement` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_movement` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_movement_template`
--

DROP TABLE IF EXISTS `creature_movement_template`;
CREATE TABLE `creature_movement_template` (
  `Entry` mediumint(8) unsigned NOT NULL COMMENT 'Creature entry',
  `PathId` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Path ID for entry',
  `Point` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `PositionX` float NOT NULL DEFAULT '0',
  `PositionY` float NOT NULL DEFAULT '0',
  `PositionZ` float NOT NULL DEFAULT '0',
  `Orientation` float NOT NULL DEFAULT '0',
  `WaitTime` int(10) unsigned NOT NULL DEFAULT '0',
  `ScriptId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Comment` text,
  PRIMARY KEY (`Entry`,`PathId`,`Point`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature waypoint system';

--
-- Dumping data for table `creature_movement_template`
--

LOCK TABLES `creature_movement_template` WRITE;
/*!40000 ALTER TABLE `creature_movement_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_movement_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_onkill_reputation`
--

DROP TABLE IF EXISTS `creature_onkill_reputation`;
CREATE TABLE `creature_onkill_reputation` (
  `creature_id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Creature Identifier',
  `RewOnKillRepFaction1` smallint(6) NOT NULL DEFAULT '0',
  `RewOnKillRepFaction2` smallint(6) NOT NULL DEFAULT '0',
  `MaxStanding1` tinyint(4) NOT NULL DEFAULT '0',
  `IsTeamAward1` tinyint(4) NOT NULL DEFAULT '0',
  `RewOnKillRepValue1` mediumint(9) NOT NULL DEFAULT '0',
  `MaxStanding2` tinyint(4) NOT NULL DEFAULT '0',
  `IsTeamAward2` tinyint(4) NOT NULL DEFAULT '0',
  `RewOnKillRepValue2` mediumint(9) NOT NULL DEFAULT '0',
  `TeamDependent` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`creature_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature OnKill Reputation gain';

--
-- Dumping data for table `creature_onkill_reputation`
--

LOCK TABLES `creature_onkill_reputation` WRITE;
/*!40000 ALTER TABLE `creature_onkill_reputation` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_onkill_reputation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_questrelation`
--

DROP TABLE IF EXISTS `creature_questrelation`;
CREATE TABLE `creature_questrelation` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature_questrelation`
--

LOCK TABLES `creature_questrelation` WRITE;
/*!40000 ALTER TABLE `creature_questrelation` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_questrelation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_template`
--

DROP TABLE IF EXISTS `creature_template`;
CREATE TABLE `creature_template` (
  `Entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Name` char(100) NOT NULL DEFAULT '',
  `SubName` char(100) DEFAULT NULL,
  `MinLevel` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `MaxLevel` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `DisplayId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `DisplayId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `DisplayId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `DisplayId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `DisplayIdProbability1` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayIdProbability2` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayIdProbability3` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `DisplayIdProbability4` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `Faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `Scale` float NOT NULL DEFAULT '1',
  `Family` tinyint(4) NOT NULL DEFAULT '0',
  `CreatureType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `InhabitType` tinyint(3) unsigned NOT NULL DEFAULT '3',
  `RegenerateStats` tinyint(3) unsigned NOT NULL DEFAULT '3',
  `RacialLeader` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `NpcFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `UnitFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `DynamicFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `ExtraFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `CreatureTypeFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `StaticFlags1` INT UNSIGNED NOT NULL DEFAULT 0,
  `StaticFlags2` INT UNSIGNED NOT NULL DEFAULT 0,
  `StaticFlags3` INT UNSIGNED NOT NULL DEFAULT 0,
  `StaticFlags4` INT UNSIGNED NOT NULL DEFAULT 0,
  `SpeedWalk` float NOT NULL DEFAULT '0',
  `SpeedRun` float NOT NULL DEFAULT '0',
  `Detection` INT(10) UNSIGNED NOT NULL DEFAULT '18' COMMENT 'Detection range for proximity',
  `CallForHelp` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Range in which creature calls for help?',
  `Pursuit` INT(10) UNSIGNED NOT NULL DEFAULT '15000' COMMENT 'Hit refresh time after which creature evades',
  `Leash` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Leash range from combat start position',
  `Timeout` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Time for refreshing leashing before evade?',
  `UnitClass` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Rank` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `HealthMultiplier` float NOT NULL DEFAULT '1',
  `PowerMultiplier` float NOT NULL DEFAULT '1',
  `DamageMultiplier` float NOT NULL DEFAULT '1',
  `DamageVariance` float NOT NULL DEFAULT '1',
  `ArmorMultiplier` float NOT NULL DEFAULT '1',
  `ExperienceMultiplier` float NOT NULL DEFAULT '1',
  `StrengthMultiplier` float NOT NULL DEFAULT '1',
  `AgilityMultiplier` float NOT NULL DEFAULT '1',
  `StaminaMultiplier` float NOT NULL DEFAULT '1',
  `IntellectMultiplier` float NOT NULL DEFAULT '1',
  `SpiritMultiplier` float NOT NULL DEFAULT '1',
  `MinLevelHealth` int(10) unsigned NOT NULL DEFAULT '0',
  `MaxLevelHealth` int(10) unsigned NOT NULL DEFAULT '0',
  `MinLevelMana` int(10) unsigned NOT NULL DEFAULT '0',
  `MaxLevelMana` int(10) unsigned NOT NULL DEFAULT '0',
  `MinMeleeDmg` float NOT NULL DEFAULT '0',
  `MaxMeleeDmg` float NOT NULL DEFAULT '0',
  `MinRangedDmg` float NOT NULL DEFAULT '0',
  `MaxRangedDmg` float NOT NULL DEFAULT '0',
  `Armor` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `MeleeAttackPower` int(10) unsigned NOT NULL DEFAULT '0',
  `RangedAttackPower` smallint(5) unsigned NOT NULL DEFAULT '0',
  `MeleeBaseAttackTime` int(10) unsigned NOT NULL DEFAULT '2000',
  `RangedBaseAttackTime` int(10) unsigned NOT NULL DEFAULT '2000',
  `DamageSchool` tinyint(4) NOT NULL DEFAULT '0',
  `MinLootGold` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `MaxLootGold` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `LootId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `PickpocketLootId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SkinningLootId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `KillCredit1` int(11) unsigned NOT NULL DEFAULT '0',
  `KillCredit2` int(11) unsigned NOT NULL DEFAULT '0',
  `MechanicImmuneMask` int(10) unsigned NOT NULL DEFAULT '0',
  `SchoolImmuneMask` int(10) unsigned NOT NULL DEFAULT '0',
  `ResistanceHoly` smallint(5) NOT NULL DEFAULT '0',
  `ResistanceFire` smallint(5) NOT NULL DEFAULT '0',
  `ResistanceNature` smallint(5) NOT NULL DEFAULT '0',
  `ResistanceFrost` smallint(5) NOT NULL DEFAULT '0',
  `ResistanceShadow` smallint(5) NOT NULL DEFAULT '0',
  `ResistanceArcane` smallint(5) NOT NULL DEFAULT '0',
  `PetSpellDataId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `MovementType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TrainerType` tinyint(4) NOT NULL DEFAULT '0',
  `TrainerSpell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `TrainerClass` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TrainerRace` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TrainerTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `VendorTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `GossipMenuId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `InteractionPauseTimer` INT(10) NOT NULL DEFAULT -1,
  `EquipmentTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Civilian` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `CorpseDecay` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Time before corpse despawns',
  `SpellList` INT NOT NULL DEFAULT '0' COMMENT 'creature_spell_list_entry',
  `CharmedSpellList` INT NOT NULL DEFAULT '0' COMMENT 'creature_spell_list_entry during charm',
  `StringId1` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `StringId2` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `AIName` char(64) NOT NULL DEFAULT '',
  `ScriptName` char(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System';

--
-- Dumping data for table `creature_template`
--

LOCK TABLES `creature_template` WRITE;
/*!40000 ALTER TABLE `creature_template` DISABLE KEYS */;
INSERT INTO `creature_template` VALUES
(1,'Waypoint (Only GM can see it)','Visual',63,63,10045,0,0,0,100,0,0,0,35,0,8,8,7,1,0,0,4096,0,130,5242886,0,0,0,0,0.91,1.14286,20,0,0,0,0,0,3,1,1,1,1,1,1,1,1,1,1,1,9999,9999,0,0,7,7,1.76,2.42,0,3,100,2000,2200,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,'','');
/*!40000 ALTER TABLE `creature_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_template_addon`
--

DROP TABLE IF EXISTS `creature_template_addon`;
CREATE TABLE `creature_template_addon` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `mount` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `stand_state` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `sheath_state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `emote` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `moveflags` int(10) unsigned NOT NULL DEFAULT '0',
  `auras` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `creature_template_addon`
--

LOCK TABLES `creature_template_addon` WRITE;
/*!40000 ALTER TABLE `creature_template_addon` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_template_addon` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `creature_template_spells`
--

DROP TABLE IF EXISTS `creature_template_spells`;
CREATE TABLE `creature_template_spells` (
  `entry` mediumint(8) unsigned NOT NULL,
  `setId` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Id of set of spells',
  `spell1` mediumint(8) unsigned NOT NULL,
  `spell2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell7` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell8` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell9` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell10` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`setId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature System (Spells used by creature)';

--
-- Dumping data for table `creature_template_spells`
--

LOCK TABLES `creature_template_spells` WRITE;
/*!40000 ALTER TABLE `creature_template_spells` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_template_spells` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS creature_cooldowns;
CREATE TABLE creature_cooldowns (
  `Entry` mediumint(8) unsigned NOT NULL,
  `SpellId` int(11) unsigned NOT NULL,
  `CooldownMin` int(10) unsigned NOT NULL,
  `CooldownMax` int(10) unsigned NOT NULL,
  PRIMARY KEY (`entry`, `SpellId`)
);

DROP TABLE IF EXISTS `creature_spell_list_entry`;
CREATE TABLE `creature_spell_list_entry` (
Id INT NOT NULL COMMENT 'List ID',
Name VARCHAR(200) NOT NULL COMMENT 'Description of usage',
ChanceSupportAction INT NOT NULL COMMENT 'Chance of support action per tick',
ChanceRangedAttack INT NOT NULL COMMENT 'Chance of ranged attack per tick',
PRIMARY KEY(Id)
);

DROP TABLE IF EXISTS `creature_spell_list`;
CREATE TABLE `creature_spell_list` (
Id INT NOT NULL COMMENT 'List ID',
Position INT NOT NULL COMMENT 'Position on list',
SpellId INT NOT NULL COMMENT 'Spell ID',
Flags INT NOT NULL COMMENT 'Spell Flags',
`CombatCondition` INT(11) NOT NULL DEFAULT -1,
TargetId INT NOT NULL COMMENT 'Targeting ID',
ScriptId INT NOT NULL COMMENT 'Dbscript to be launched on success',
Availability INT NOT NULL COMMENT 'Chance on spawn for spell to be included',
Probability INT NOT NULL COMMENT 'Weight of spell when multiple are available',
InitialMin INT NOT NULL COMMENT 'Initial delay minimum',
InitialMax INT NOT NULL COMMENT 'Initial delay maximum',
RepeatMin INT NOT NULL COMMENT 'Repeated delay minimum',
RepeatMax INT NOT NULL COMMENT 'Repeated delay maximum',
Comments VARCHAR(255) NOT NULL COMMENT 'Description of spell use',
PRIMARY KEY(Id, Position)
);

DROP TABLE IF EXISTS `creature_spell_targeting`;
CREATE TABLE `creature_spell_targeting` (
Id INT NOT NULL COMMENT 'Targeting ID',
Type INT NOT NULL COMMENT 'Type of targeting ID',
Param1 INT NOT NULL COMMENT 'First parameter',
Param2 INT NOT NULL COMMENT 'Second parameter',
Param3 INT NOT NULL COMMENT 'Third parameter',
`UnitCondition` INT(11) NOT NULL DEFAULT -1,
Comments VARCHAR(255) NOT NULL COMMENT 'Description of target',
PRIMARY KEY(Id)
);

LOCK TABLES `creature_spell_targeting` WRITE;
INSERT INTO `creature_spell_targeting`(Id, Type, Param1, Param2, Param3, Comments) VALUES
(0, 0, 0, 0, 0, 'Hardcoded - none'),
(1, 0, 0, 0, 0, 'Hardcoded - current'),
(2, 0, 0, 0, 0, 'Hardcoded - self'),
(3, 0, 0, 0, 0, 'Hardcoded - eligible friendly dispel'),
(4, 0, 0, 0, 0, 'Hardcoded - eligible friendly dispel - skip self'),
(100, 1, 0, 0, 0x0002, 'Attack - random player');
UNLOCK TABLES;

--
-- Table structure for table `creature_immunities`
--

DROP TABLE IF EXISTS `creature_immunities`;
CREATE TABLE `creature_immunities`(
`Entry` INT UNSIGNED NOT NULL COMMENT 'creature_template entry',
`SetId` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'immunity set ID',
`Type` TINYINT UNSIGNED NOT NULL COMMENT 'enum SpellImmunity',
`Value` INT UNSIGNED NOT NULL COMMENT 'value depending on type',
PRIMARY KEY(`Entry`,`SetId`,`Type`,`Value`)
);

--
-- Table structure for table `custom_texts`
--

DROP TABLE IF EXISTS `custom_texts`;
CREATE TABLE `custom_texts` (
  `entry` mediumint(8) NOT NULL,
  `content_default` text NOT NULL,
  `content_loc1` text,
  `content_loc2` text,
  `content_loc3` text,
  `content_loc4` text,
  `content_loc5` text,
  `content_loc6` text,
  `content_loc7` text,
  `content_loc8` text,
  `sound` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `language` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `emote` smallint(5) unsigned NOT NULL DEFAULT '0',
  `comment` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Custom Texts';

--
-- Dumping data for table `custom_texts`
--

LOCK TABLES `custom_texts` WRITE;
/*!40000 ALTER TABLE `custom_texts` DISABLE KEYS */;
/*!40000 ALTER TABLE `custom_texts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dbscripts_on_creature_movement`
--

DROP TABLE IF EXISTS `dbscripts_on_creature_movement`;
CREATE TABLE `dbscripts_on_creature_movement` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `delay` int(10) unsigned NOT NULL DEFAULT '0',
  `priority` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `command` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `datalong` int(10) unsigned NOT NULL DEFAULT '0',
  `datalong2` int(10) unsigned NOT NULL DEFAULT '0',
  `datalong3` int(11) unsigned NOT NULL DEFAULT '0',
  `buddy_entry` int(10) unsigned NOT NULL DEFAULT '0',
  `search_radius` int(10) unsigned NOT NULL DEFAULT '0',
  `data_flags` int unsigned NOT NULL DEFAULT '0',
  `dataint` int(11) NOT NULL DEFAULT '0',
  `dataint2` int(11) NOT NULL DEFAULT '0',
  `dataint3` int(11) NOT NULL DEFAULT '0',
  `dataint4` int(11) NOT NULL DEFAULT '0',
  `datafloat` float NOT NULL DEFAULT '0',
  `x` float NOT NULL DEFAULT '0',
  `y` float NOT NULL DEFAULT '0',
  `z` float NOT NULL DEFAULT '0',
  `o` float NOT NULL DEFAULT '0',
  `speed` float NOT NULL DEFAULT '0',
  `condition_id` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
  `comments` varchar(255) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dbscripts_on_creature_movement`
--

LOCK TABLES `dbscripts_on_creature_movement` WRITE;
/*!40000 ALTER TABLE `dbscripts_on_creature_movement` DISABLE KEYS */;
/*!40000 ALTER TABLE `dbscripts_on_creature_movement` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure of `dbscripts_on_event`, `dbscripts_on_go_use`, `dbscripts_on_go_template_use`,
--                    `dbscripts_on_gossip`, `dbscripts_on_quest_end`, `dbscripts_on_quest_start`,
--                    `dbscripts_on_spell`, `dbscripts_on_creature_death`
DROP TABLE IF EXISTS dbscripts_on_event;
CREATE TABLE dbscripts_on_event LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_go_use;
CREATE TABLE dbscripts_on_go_use LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_go_template_use;
CREATE TABLE dbscripts_on_go_template_use LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_gossip;
CREATE TABLE dbscripts_on_gossip LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_quest_end;
CREATE TABLE dbscripts_on_quest_end LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_quest_start;
CREATE TABLE dbscripts_on_quest_start LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_spell;
CREATE TABLE dbscripts_on_spell LIKE dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_creature_death;
CREATE TABLE dbscripts_on_creature_death LIKE dbscripts_on_creature_movement;

--
-- Table structure for table `dbscript_random_templates`
--

DROP TABLE IF EXISTS `dbscript_random_templates`;
CREATE TABLE `dbscript_random_templates` (
  `id` int(11) unsigned NOT NULL COMMENT 'Id of template',
  `type` int(11) unsigned NOT NULL COMMENT 'Type of template',
  `target_id` int(11) NOT NULL DEFAULT '0' COMMENT 'Id of chanced element',
  `chance` int(11) NOT NULL DEFAULT '0' COMMENT 'Chance for element to occur in %',
  `comments` VARCHAR(500) DEFAULT '',
  PRIMARY KEY (`id`,`type`,`target_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='DBScript system';

--
-- Table structure for table `disenchant_loot_template`
--

DROP TABLE IF EXISTS `disenchant_loot_template`;
CREATE TABLE `disenchant_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Recommended id selection: item_level*100 + item_quality',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `disenchant_loot_template`
--

LOCK TABLES `disenchant_loot_template` WRITE;
/*!40000 ALTER TABLE `disenchant_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `disenchant_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `exploration_basexp`
--

DROP TABLE IF EXISTS `exploration_basexp`;
CREATE TABLE `exploration_basexp` (
  `level` tinyint(4) NOT NULL DEFAULT '0',
  `basexp` mediumint(9) NOT NULL DEFAULT '0',
  PRIMARY KEY (`level`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Exploration System';

--
-- Dumping data for table `exploration_basexp`
--

LOCK TABLES `exploration_basexp` WRITE;
/*!40000 ALTER TABLE `exploration_basexp` DISABLE KEYS */;
INSERT INTO `exploration_basexp` VALUES
(0,0),
(1,5),
(2,15),
(3,25),
(4,35),
(5,45),
(6,55),
(7,65),
(8,70),
(9,80),
(10,85),
(11,90),
(12,90),
(13,90),
(14,100),
(15,105),
(16,115),
(17,125),
(18,135),
(19,145),
(20,155),
(21,165),
(22,175),
(23,185),
(24,195),
(25,200),
(26,210),
(27,220),
(28,230),
(29,240),
(30,245),
(31,250),
(32,255),
(33,265),
(34,270),
(35,275),
(36,280),
(37,285),
(38,285),
(39,300),
(40,315),
(41,330),
(42,345),
(43,360),
(44,375),
(45,390),
(46,405),
(47,420),
(48,440),
(49,455),
(50,470),
(51,490),
(52,510),
(53,530),
(54,540),
(55,560),
(56,580),
(57,600),
(58,620),
(59,640),
(60,660);
/*!40000 ALTER TABLE `exploration_basexp` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fishing_loot_template`
--

DROP TABLE IF EXISTS `fishing_loot_template`;
CREATE TABLE `fishing_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'entry 0 used for junk loot at fishing fail (if allowed by config option)',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `fishing_loot_template`
--

LOCK TABLES `fishing_loot_template` WRITE;
/*!40000 ALTER TABLE `fishing_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `fishing_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_event`
--

DROP TABLE IF EXISTS `game_event`;
CREATE TABLE `game_event` (
  `entry` mediumint(8) unsigned NOT NULL COMMENT 'Entry of the game event',
  `occurence` bigint(20) unsigned NOT NULL DEFAULT '86400' COMMENT 'Delay in minutes between occurences of the event',
  `length` bigint(20) unsigned NOT NULL DEFAULT '43200' COMMENT 'Length in minutes of the event',
  `holiday` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Client side holiday id',
  `linkedTo` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'This event starts only if defined LinkedTo event is started',
  `EventGroup` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Used for events that are randomized daily/weekly',
  `description` varchar(255) DEFAULT NULL COMMENT 'Description of the event displayed in console',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `game_event`
--

LOCK TABLES `game_event` WRITE;
/*!40000 ALTER TABLE `game_event` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS game_event_time;
CREATE TABLE `game_event_time` (
  `entry` mediumint(8) unsigned NOT NULL COMMENT 'Entry of the game event',
  `start_time` DATETIME NOT NULL DEFAULT '1970-01-01 00:00:00' COMMENT 'Absolute start date, the event will never start before',
  `end_time` DATETIME NOT NULL DEFAULT '1970-01-01 00:00:00' COMMENT 'Absolute end date, the event will never start after',
    PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `game_event_creature`
--

DROP TABLE IF EXISTS `game_event_creature`;
CREATE TABLE `game_event_creature` (
  `guid` int(10) unsigned NOT NULL,
  `event` smallint(6) NOT NULL DEFAULT '0' COMMENT 'Negatives value to remove during event and ignore pool grouping, positive value for spawn during event and if guid is part of pool then al pool memebers must be listed as part of event spawn.',
  PRIMARY KEY (`guid`,`event`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `game_event_creature`
--

LOCK TABLES `game_event_creature` WRITE;
/*!40000 ALTER TABLE `game_event_creature` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event_creature` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_event_creature_data`
--

DROP TABLE IF EXISTS `game_event_creature_data`;
CREATE TABLE `game_event_creature_data` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `entry_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `modelid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `equipment_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell_start` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell_end` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `event` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`event`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `game_event_creature_data`
--

LOCK TABLES `game_event_creature_data` WRITE;
/*!40000 ALTER TABLE `game_event_creature_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event_creature_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_event_gameobject`
--

DROP TABLE IF EXISTS `game_event_gameobject`;
CREATE TABLE `game_event_gameobject` (
  `guid` int(10) unsigned NOT NULL,
  `event` smallint(6) NOT NULL DEFAULT '0' COMMENT 'Negatives value to remove during event and ignore pool grouping, positive value for spawn during event and if guid is part of pool then al pool memebers must be listed as part of event spawn.',
  PRIMARY KEY (`guid`,`event`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `game_event_gameobject`
--

LOCK TABLES `game_event_gameobject` WRITE;
/*!40000 ALTER TABLE `game_event_gameobject` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event_gameobject` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_event_mail`
--

DROP TABLE IF EXISTS `game_event_mail`;
CREATE TABLE `game_event_mail` (
  `event` smallint(6) NOT NULL DEFAULT '0' COMMENT 'Negatives value to send at event stop, positive value for send at event start.',
  `raceMask` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `mailTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `senderEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event`,`raceMask`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Game event system';

--
-- Dumping data for table `game_event_mail`
--

LOCK TABLES `game_event_mail` WRITE;
/*!40000 ALTER TABLE `game_event_mail` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event_mail` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_event_quest`
--

DROP TABLE IF EXISTS `game_event_quest`;
CREATE TABLE `game_event_quest` (
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'entry from quest_template',
  `event` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'entry from game_event',
  PRIMARY KEY (`quest`,`event`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Game event system';

--
-- Dumping data for table `game_event_quest`
--

LOCK TABLES `game_event_quest` WRITE;
/*!40000 ALTER TABLE `game_event_quest` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_event_quest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_graveyard_zone`
--

DROP TABLE IF EXISTS `game_graveyard_zone`;
CREATE TABLE `game_graveyard_zone` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ghost_loc` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `link_kind` tinyint unsigned NOT NULL DEFAULT '0',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`ghost_loc`, `link_kind`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Graveyard link definitions';

--
-- Dumping data for table `game_graveyard_zone`
--

LOCK TABLES `game_graveyard_zone` WRITE;
/*!40000 ALTER TABLE `game_graveyard_zone` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_graveyard_zone` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_tele`
--

DROP TABLE IF EXISTS `game_tele`;
CREATE TABLE `game_tele` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Tele Command';

--
-- Dumping data for table `game_tele`
--

LOCK TABLES `game_tele` WRITE;
/*!40000 ALTER TABLE `game_tele` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_tele` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_weather`
--

DROP TABLE IF EXISTS `game_weather`;
CREATE TABLE `game_weather` (
  `zone` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spring_rain_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `spring_snow_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `spring_storm_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `summer_rain_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `summer_snow_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `summer_storm_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `fall_rain_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `fall_snow_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `fall_storm_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `winter_rain_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `winter_snow_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  `winter_storm_chance` tinyint(3) unsigned NOT NULL DEFAULT '25',
  PRIMARY KEY (`zone`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Weather System';

--
-- Dumping data for table `game_weather`
--

LOCK TABLES `game_weather` WRITE;
/*!40000 ALTER TABLE `game_weather` DISABLE KEYS */;
/*!40000 ALTER TABLE `game_weather` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gameobject`
--

DROP TABLE IF EXISTS `gameobject`;
CREATE TABLE `gameobject` (
  `guid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Global Unique Identifier',
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Gameobject Identifier',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `spawnMask` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `position_x` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `position_y` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `position_z` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `orientation` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `rotation0` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `rotation1` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `rotation2` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `rotation3` DECIMAL(40,20) NOT NULL DEFAULT '0',
  `spawntimesecsmin` int(11) NOT NULL DEFAULT '0' COMMENT 'GameObject respawn time minimum',
  `spawntimesecsmax` int(11) NOT NULL DEFAULT '0' COMMENT 'Gameobject respawn time maximum',
  PRIMARY KEY (`guid`),
  KEY `idx_map` (`map`),
  KEY `idx_id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Gameobject System';

--
-- Dumping data for table `gameobject`
--

LOCK TABLES `gameobject` WRITE;
/*!40000 ALTER TABLE `gameobject` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS `gameobject_addon`;
CREATE TABLE `gameobject_addon` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `animprogress` TINYINT(3) UNSIGNED NOT NULL DEFAULT '100',
  `state` TINYINT(3) NOT NULL DEFAULT -1,
  `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY(`guid`)
);

DROP TABLE IF EXISTS `gameobject_spawn_entry`;
CREATE TABLE `gameobject_spawn_entry`(
`guid` INT UNSIGNED NOT NULL COMMENT 'Gameobject table guid',
`entry` MEDIUMINT UNSIGNED NOT NULL COMMENT 'Gameobject Template entry',
PRIMARY KEY(`guid`,`entry`)
);

--
-- Table structure for table `gameobject_battleground`
--

DROP TABLE IF EXISTS `gameobject_battleground`;
CREATE TABLE `gameobject_battleground` (
  `guid` int(10) unsigned NOT NULL COMMENT 'GameObject''s GUID',
  `event1` tinyint(3) unsigned NOT NULL COMMENT 'main event',
  `event2` tinyint(3) unsigned NOT NULL COMMENT 'sub event',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='GameObject battleground indexing system';

--
-- Dumping data for table `gameobject_battleground`
--

LOCK TABLES `gameobject_battleground` WRITE;
/*!40000 ALTER TABLE `gameobject_battleground` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject_battleground` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gameobject_involvedrelation`
--

DROP TABLE IF EXISTS `gameobject_involvedrelation`;
CREATE TABLE `gameobject_involvedrelation` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gameobject_involvedrelation`
--

LOCK TABLES `gameobject_involvedrelation` WRITE;
/*!40000 ALTER TABLE `gameobject_involvedrelation` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject_involvedrelation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gameobject_loot_template`
--

DROP TABLE IF EXISTS `gameobject_loot_template`;
CREATE TABLE `gameobject_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `gameobject_loot_template`
--

LOCK TABLES `gameobject_loot_template` WRITE;
/*!40000 ALTER TABLE `gameobject_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gameobject_questrelation`
--

DROP TABLE IF EXISTS `gameobject_questrelation`;
CREATE TABLE `gameobject_questrelation` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gameobject_questrelation`
--

LOCK TABLES `gameobject_questrelation` WRITE;
/*!40000 ALTER TABLE `gameobject_questrelation` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject_questrelation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gameobject_template`
--

DROP TABLE IF EXISTS `gameobject_template`;
CREATE TABLE `gameobject_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `displayId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL DEFAULT '',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `ExtraFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `size` float NOT NULL DEFAULT '1',
  `data0` int(10) unsigned NOT NULL DEFAULT '0',
  `data1` int(10) NOT NULL DEFAULT '0',
  `data2` int(10) unsigned NOT NULL DEFAULT '0',
  `data3` int(10) unsigned NOT NULL DEFAULT '0',
  `data4` int(10) unsigned NOT NULL DEFAULT '0',
  `data5` int(10) unsigned NOT NULL DEFAULT '0',
  `data6` int(10) NOT NULL DEFAULT '0',
  `data7` int(10) unsigned NOT NULL DEFAULT '0',
  `data8` int(10) unsigned NOT NULL DEFAULT '0',
  `data9` int(10) unsigned NOT NULL DEFAULT '0',
  `data10` int(10) unsigned NOT NULL DEFAULT '0',
  `data11` int(10) unsigned NOT NULL DEFAULT '0',
  `data12` int(10) unsigned NOT NULL DEFAULT '0',
  `data13` int(10) unsigned NOT NULL DEFAULT '0',
  `data14` int(10) unsigned NOT NULL DEFAULT '0',
  `data15` int(10) unsigned NOT NULL DEFAULT '0',
  `data16` int(10) unsigned NOT NULL DEFAULT '0',
  `data17` int(10) unsigned NOT NULL DEFAULT '0',
  `data18` int(10) unsigned NOT NULL DEFAULT '0',
  `data19` int(10) unsigned NOT NULL DEFAULT '0',
  `data20` int(10) unsigned NOT NULL DEFAULT '0',
  `data21` int(10) unsigned NOT NULL DEFAULT '0',
  `data22` int(10) unsigned NOT NULL DEFAULT '0',
  `data23` int(10) unsigned NOT NULL DEFAULT '0',
  `CustomData1` int(10) unsigned NOT NULL DEFAULT '0',
  `mingold` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `maxgold` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `ScriptName` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Gameobject System';

--
-- Dumping data for table `gameobject_template`
--

LOCK TABLES `gameobject_template` WRITE;
/*!40000 ALTER TABLE `gameobject_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `gameobject_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gossip_menu`
--

DROP TABLE IF EXISTS `gossip_menu`;
CREATE TABLE `gossip_menu` (
  `entry` smallint(6) unsigned NOT NULL DEFAULT '0',
  `text_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `script_id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'script in `dbscripts_on_gossip` - will be executed on GossipHello',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`text_id`,`script_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gossip_menu`
--

LOCK TABLES `gossip_menu` WRITE;
/*!40000 ALTER TABLE `gossip_menu` DISABLE KEYS */;
/*!40000 ALTER TABLE `gossip_menu` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gossip_menu_option`
--

DROP TABLE IF EXISTS `gossip_menu_option`;
CREATE TABLE `gossip_menu_option` (
  `menu_id` smallint(6) unsigned NOT NULL DEFAULT '0',
  `id` smallint(6) unsigned NOT NULL DEFAULT '0',
  `option_icon` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `option_text` text,
  `option_broadcast_text` INT(11) NOT NULL DEFAULT '0',
  `option_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `npc_option_npcflag` int(10) unsigned NOT NULL DEFAULT '0',
  `action_menu_id` mediumint(8) NOT NULL DEFAULT '0',
  `action_poi_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `action_script_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `box_coded` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `box_money` int(11) unsigned NOT NULL DEFAULT '0',
  `box_text` text,
  `box_broadcast_text` INT(11) NOT NULL,
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`menu_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `gossip_menu_option`
--

LOCK TABLES `gossip_menu_option` WRITE;
/*!40000 ALTER TABLE `gossip_menu_option` DISABLE KEYS */;
INSERT INTO `gossip_menu_option`(menu_id, id, option_icon, option_text, option_id, npc_option_npcflag, action_menu_id, action_poi_id, action_script_id, box_coded, box_money, box_text, condition_id) VALUES
(0,0,0,'GOSSIP_OPTION_QUESTGIVER',2,2,0,0,0,0,0,NULL,0),
(0,1,1,'GOSSIP_OPTION_VENDOR',3,4,0,0,0,0,0,NULL,0),
(0,2,2,'GOSSIP_OPTION_TAXIVENDOR',4,8,0,0,0,0,0,NULL,0),
(0,3,3,'GOSSIP_OPTION_TRAINER',5,16,0,0,0,0,0,NULL,0),
(0,4,4,'GOSSIP_OPTION_SPIRITHEALER',6,32,0,0,0,0,0,NULL,0),
(0,5,4,'GOSSIP_OPTION_SPIRITGUIDE',7,64,0,0,0,0,0,NULL,0),
(0,6,5,'GOSSIP_OPTION_INNKEEPER',8,128,0,0,0,0,0,NULL,0),
(0,7,6,'GOSSIP_OPTION_BANKER',9,256,0,0,0,0,0,NULL,0),
(0,8,7,'GOSSIP_OPTION_PETITIONER',10,512,0,0,0,0,0,NULL,0),
(0,9,8,'GOSSIP_OPTION_TABARDDESIGNER',11,1024,0,0,0,0,0,NULL,0),
(0,10,9,'GOSSIP_OPTION_BATTLEFIELD',12,2048,0,0,0,0,0,NULL,0),
(0,11,6,'GOSSIP_OPTION_AUCTIONEER',13,4096,0,0,0,0,0,NULL,0),
(0,12,0,'GOSSIP_OPTION_STABLEPET',14,8192,0,0,0,0,0,NULL,0),
(0,13,1,'GOSSIP_OPTION_ARMORER',15,16384,0,0,0,0,0,NULL,0),
(0,14,0,'GOSSIP_OPTION_UNLEARNTALENTS',16,16,0,0,0,0,0,NULL,0),
(0,15,2,'GOSSIP_OPTION_UNLEARNPETSKILLS',17,16,0,0,0,0,0,NULL,0),
(0,16,0,'GOSSIP_OPTION_BOT',99,1,0,0,0,0,0,NULL,0);
/*!40000 ALTER TABLE `gossip_menu_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `gossip_texts`
--

DROP TABLE IF EXISTS `gossip_texts`;
CREATE TABLE `gossip_texts` (
  `entry` mediumint(8) NOT NULL,
  `content_default` text NOT NULL,
  `content_loc1` text,
  `content_loc2` text,
  `content_loc3` text,
  `content_loc4` text,
  `content_loc5` text,
  `content_loc6` text,
  `content_loc7` text,
  `content_loc8` text,
  `comment` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Gossip Texts';

--
-- Dumping data for table `gossip_texts`
--

LOCK TABLES `gossip_texts` WRITE;
/*!40000 ALTER TABLE `gossip_texts` DISABLE KEYS */;
/*!40000 ALTER TABLE `gossip_texts` ENABLE KEYS */;
UNLOCK TABLES;

-- Table structure for table `instance_encounters`
--

DROP TABLE IF EXISTS `instance_encounters`;
CREATE TABLE `instance_encounters` (
  `entry` int(10) unsigned NOT NULL COMMENT 'Unique entry from DungeonEncounter.dbc',
  `creditType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `creditEntry` int(10) unsigned NOT NULL DEFAULT '0',
  `lastEncounterDungeon` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'If not 0, LfgDungeon.dbc entry for the instance it is last encounter in',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `instance_encounters`
--

LOCK TABLES `instance_encounters` WRITE;
/*!40000 ALTER TABLE `instance_encounters` DISABLE KEYS */;
INSERT INTO `instance_encounters` VALUES
(161,0,644,0),
(162,0,643,0),
(163,0,1763,0),
(164,0,646,0),
(165,0,645,0),
(166,0,647,0),
(167,0,639,6),
(219,0,4887,0),
(220,0,4831,0),
(221,0,6243,0),
(222,0,12902,0),
(224,0,4830,0),
(225,0,4832,0),
(226,0,4829,10),
(227,0,9018,30),
(228,0,9025,0),
(229,0,9319,0),
(230,0,10096,0),
(231,0,9024,0),
(232,0,9017,0),
(233,0,9041,0),
(234,0,9056,0),
(235,0,9016,0),
(236,0,9033,0),
(237,0,8983,0),
(238,0,9537,0),
(239,0,9502,0),
(240,0,9543,0),
(241,0,9499,0),
(242,0,9156,0),
(243,0,9035,0),
(244,0,9938,0),
(245,0,9019,276),
(267,0,9196,0),
(268,0,9236,0),
(269,0,9237,0),
(270,0,10596,0),
(271,0,10584,0),
(272,0,9736,0),
(273,0,10268,0),
(274,0,10220,0),
(275,0,9568,32),
(276,0,9816,0),
(277,0,10264,0),
(278,0,10429,0),
(279,0,10430,0),
(280,0,10363,44),
(296,1,58630,209),
(300,1,58630,210),
(334,1,68572,0),
(336,1,68572,0),
(338,1,68574,0),
(339,1,68574,0),
(340,1,68663,245),
(341,1,68663,249),
(343,0,11490,0),
(344,0,13280,0),
(345,0,14327,0),
(346,0,11492,34),
(347,0,11488,0),
(348,0,11487,0),
(349,0,11496,0),
(350,0,11489,0),
(361,0,11486,36),
(362,0,14326,0),
(363,0,14322,0),
(364,0,14321,0),
(365,0,14323,0),
(366,0,14325,0),
(367,0,14324,0),
(368,0,11501,38),
(375,1,61863,214),
(376,1,61863,215),
(378,0,7079,0),
(379,0,7361,0),
(380,0,6235,0),
(381,0,6229,0),
(382,0,7800,14),
(422,0,13282,0),
(423,0,12258,26),
(424,0,12236,272),
(425,0,12225,0),
(426,0,12203,0),
(427,0,13601,0),
(428,0,13596,0),
(429,0,12201,273),
(430,0,11517,0),
(431,0,11520,4),
(432,0,11518,0),
(433,0,11519,0),
(434,0,7355,0),
(435,0,7357,0),
(436,0,8567,0),
(437,0,7358,20),
(438,0,6168,0),
(439,0,4424,0),
(440,0,4428,0),
(441,0,4420,0),
(443,0,4421,16),
(444,0,3983,0),
(445,0,4543,18),
(446,0,3974,0),
(447,0,6487,165),
(448,0,3975,163),
(449,0,4542,0),
(450,0,3977,164),
(451,0,10506,0),
(452,0,10503,0),
(453,0,11622,0),
(454,0,10433,0),
(455,0,10432,0),
(456,0,10508,0),
(457,0,10505,0),
(458,0,11261,0),
(459,0,10901,0),
(460,0,10507,0),
(461,0,10504,0),
(462,0,10502,0),
(463,0,1853,2),
(464,0,3914,0),
(465,0,3886,0),
(466,0,3887,0),
(467,0,4278,0),
(468,0,4279,0),
(469,0,4274,0),
(470,0,3927,0),
(471,0,4275,8),
(472,0,10516,0),
(473,0,10558,0),
(474,0,10808,0),
(475,0,10997,0),
(476,0,11032,0),
(477,0,10811,0),
(478,0,10813,40),
(479,0,10436,0),
(480,0,10437,0),
(481,0,10438,0),
(482,0,10435,0),
(483,0,10439,0),
(484,0,10440,274),
(485,0,8580,0),
(486,0,5721,0),
(487,0,5720,0),
(488,0,5710,0),
(490,0,5719,0),
(491,0,5722,0),
(492,0,8443,0),
(493,0,5709,28),
(536,0,1696,0),
(537,0,1666,0),
(538,0,1717,0),
(539,0,1716,0),
(540,0,1663,12),
(547,0,6910,0),
(548,0,6906,0),
(549,0,7228,0),
(551,0,7206,0),
(552,0,7291,0),
(553,0,4854,0),
(554,0,2748,22),
(567,1,59046,0),
(568,1,59046,0),
(585,0,3671,0),
(586,0,3669,0),
(587,0,3653,0),
(588,0,3670,0),
(589,0,3674,0),
(590,0,3673,0),
(591,0,5775,0),
(592,0,3654,1),
(593,0,7795,0),
(594,0,7273,0),
(595,0,8127,0),
(596,0,7272,0),
(597,0,7271,0),
(598,0,7796,0),
(599,0,7275,0),
(600,0,7267,24),
(610,0,12435,0),
(611,0,13020,0),
(612,0,12017,0),
(613,0,11983,0),
(614,0,14601,0),
(615,0,11981,0),
(616,0,14020,0),
(617,0,11583,50),
(637,1,68184,0),
(638,1,68184,0),
(639,1,68184,0),
(640,1,68184,0),
(663,0,12118,0),
(664,0,11982,0),
(665,0,12259,0),
(666,0,12057,0),
(667,0,12264,0),
(668,0,12056,0),
(669,0,12098,0),
(670,0,11988,0),
(671,0,12018,0),
(672,0,11502,48),
(673,0,15956,0),
(674,0,15956,0),
(677,0,15953,0),
(678,0,15953,0),
(679,0,15952,0),
(680,0,15952,0),
(681,0,15954,0),
(682,0,15954,0),
(683,0,15936,0),
(684,0,15936,0),
(685,0,16011,0),
(686,0,16011,0),
(687,0,16061,0),
(689,0,16061,0),
(690,0,16060,0),
(691,0,16060,0),
(692,1,59450,0),
(693,1,59450,0),
(694,0,16028,0),
(695,0,16028,0),
(696,0,15931,0),
(697,0,15931,0),
(698,0,15932,0),
(699,0,15932,0),
(700,0,15928,0),
(701,0,15928,0),
(702,0,15989,0),
(703,0,15989,0),
(704,0,15990,159),
(706,0,15990,227),
(707,0,10184,46),
(708,0,10184,257),
(709,0,15263,0),
(710,0,15544,0),
(711,0,15516,0),
(712,0,15510,0),
(713,0,15299,0),
(714,0,15509,0),
(715,0,15275,0),
(716,0,15517,0),
(717,0,15727,161),
(718,0,15348,0),
(719,0,15341,0),
(720,0,15340,0),
(721,0,15370,0),
(722,0,15369,0),
(723,0,15339,160),
(748,1,65195,0),
(751,1,64899,0),
(752,1,64985,0),
(753,1,65074,0),
(762,1,65195,0),
(765,1,64899,0),
(766,1,64985,0),
(767,1,65074,0),
(784,0,14507,0),
(785,0,14517,0),
(786,0,14510,0),
(787,0,11382,0),
(788,0,15083,0),
(789,0,14509,0),
(790,0,15114,0),
(791,0,14515,0),
(792,0,11380,0),
(793,0,14834,42),
(843,1,72830,255),
(844,1,72830,256),
(847,1,72959,0),
(854,1,72706,0),
(859,1,72959,0),
(866,1,72706,0),
(883,0,4422,0);
/*!40000 ALTER TABLE `instance_encounters` ENABLE KEYS */;
UNLOCK TABLES;

-- Table structure for table `instance_template`
--

DROP TABLE IF EXISTS `instance_template`;
CREATE TABLE `instance_template` (
  `map` smallint(5) unsigned NOT NULL,
  `parent` smallint(5) unsigned NOT NULL DEFAULT '0',
  `levelMin` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `levelMax` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `maxPlayers` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `reset_delay` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Reset time in days',
  `ghostEntranceMap` smallint(5) unsigned NOT NULL,
  `ghostEntranceX` float NOT NULL,
  `ghostEntranceY` float NOT NULL,
  `ScriptName` varchar(128) NOT NULL DEFAULT '',
  `mountAllowed` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `instance_template`
--

LOCK TABLES `instance_template` WRITE;
/*!40000 ALTER TABLE `instance_template` DISABLE KEYS */;
INSERT INTO `instance_template` VALUES
(30, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(33,0,20,26,10,0,0,-230.989,1571.57,'',0),
(34,0,24,32,10,0,0,-8762.38,848.01,'',0),
(36,0,17,26,10,0,0,-11207.8,1681.15,'',1),
(43,0,17,24,10,0,1,-751.131,-2209.24,'',0),
(47,0,29,38,10,0,1,-4459.45,-1660.21,'',0),
(48,0,24,32,10,0,1,4249.12,748.387,'',0),
(70,0,41,51,10,0,0,-6060.18,-2955,'',0),
(90,0,29,38,10,0,0,-5162.66,931.599,'',0),
(109,0,50,0,10,0,0,-10170.1,-3995.97,'',0),
(129,0,37,46,10,0,1,-4662.88,-2535.87,'',0),
(189,0,34,45,10,0,0,2892.24,-811.264,'',0),
(209,0,44,54,10,0,1,-6790.58,-2891.28,'',1),
(229,0,55,0,10,3,0,-7522.53,-1233.04,'',0),
(230,0,52,0,5,0,0,-7178.1,-928.639,'',0),
(249,0,60,0,40,5,1,-4753.31,-3752.42,'',0),
(289,0,58,0,5,0,0,1274.78,-2552.56,'',0),
(309,0,60,0,20,3,0,-11916.1,-1224.58,'',1),
(329,0,58,0,5,0,0,3392.32,-3378.48,'',0),
(349,0,46,55,10,0,1,-1432.7,2924.98,'',0),
(369, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(389,0,13,18,10,0,1,1816.76,-4423.37,'',0),
(409,0,60,0,40,7,0,-7510.56,-1036.7,'',0),
(429,0,55,0,5,0,1,-3908.03,1130,'',0),
(449, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(450, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(469,0,60,0,40,7,0,-7663.41,-1218.67,'',0),
(489, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(509,0,60,0,20,3,1,-8114.46,1526.37,'',1),
(529, 0, 0, 0, 0, 0, 0, 0, 0, '', 0),
(531,0,60,0,40,7,1,-8111.72,1526.79,'',0),
(533,0,60,0,40,7,0,0,0,'',0);
/*!40000 ALTER TABLE `instance_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_enchantment_template`
--

DROP TABLE IF EXISTS `item_enchantment_template`;
CREATE TABLE `item_enchantment_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ench` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`ench`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item Random Enchantment System';

--
-- Dumping data for table `item_enchantment_template`
--

LOCK TABLES `item_enchantment_template` WRITE;
/*!40000 ALTER TABLE `item_enchantment_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_enchantment_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_loot_template`
--

DROP TABLE IF EXISTS `item_loot_template`;
CREATE TABLE `item_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `item_loot_template`
--

LOCK TABLES `item_loot_template` WRITE;
/*!40000 ALTER TABLE `item_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_required_target`
--

DROP TABLE IF EXISTS `item_required_target`;
CREATE TABLE `item_required_target` (
  `entry` mediumint(8) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `targetEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `entry_type_target` (`entry`,`type`,`targetEntry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `item_required_target`
--

LOCK TABLES `item_required_target` WRITE;
/*!40000 ALTER TABLE `item_required_target` DISABLE KEYS */;
/*!40000 ALTER TABLE `item_required_target` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `item_template`
--

DROP TABLE IF EXISTS `item_template`;
CREATE TABLE `item_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `subclass` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  `displayid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Quality` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `BuyCount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `BuyPrice` int(10) unsigned NOT NULL DEFAULT '0',
  `SellPrice` int(10) unsigned NOT NULL DEFAULT '0',
  `InventoryType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AllowableClass` mediumint(9) NOT NULL DEFAULT '-1',
  `AllowableRace` mediumint(9) NOT NULL DEFAULT '-1',
  `ItemLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RequiredLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RequiredSkill` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredSkillRank` smallint(5) unsigned NOT NULL DEFAULT '0',
  `requiredspell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `requiredhonorrank` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RequiredCityRank` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RequiredReputationFaction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredReputationRank` smallint(5) unsigned NOT NULL DEFAULT '0',
  `maxcount` smallint(5) unsigned NOT NULL DEFAULT '0',
  `stackable` smallint(5) unsigned NOT NULL DEFAULT '1',
  `ContainerSlots` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_type1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value1` smallint(6) NOT NULL DEFAULT '0',
  `stat_type2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value2` smallint(6) NOT NULL DEFAULT '0',
  `stat_type3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value3` smallint(6) NOT NULL DEFAULT '0',
  `stat_type4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value4` smallint(6) NOT NULL DEFAULT '0',
  `stat_type5` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value5` smallint(6) NOT NULL DEFAULT '0',
  `stat_type6` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value6` smallint(6) NOT NULL DEFAULT '0',
  `stat_type7` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value7` smallint(6) NOT NULL DEFAULT '0',
  `stat_type8` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value8` smallint(6) NOT NULL DEFAULT '0',
  `stat_type9` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value9` smallint(6) NOT NULL DEFAULT '0',
  `stat_type10` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stat_value10` smallint(6) NOT NULL DEFAULT '0',
  `dmg_min1` float NOT NULL DEFAULT '0',
  `dmg_max1` float NOT NULL DEFAULT '0',
  `dmg_type1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `dmg_min2` float NOT NULL DEFAULT '0',
  `dmg_max2` float NOT NULL DEFAULT '0',
  `dmg_type2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `dmg_min3` float NOT NULL DEFAULT '0',
  `dmg_max3` float NOT NULL DEFAULT '0',
  `dmg_type3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `dmg_min4` float NOT NULL DEFAULT '0',
  `dmg_max4` float NOT NULL DEFAULT '0',
  `dmg_type4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `dmg_min5` float NOT NULL DEFAULT '0',
  `dmg_max5` float NOT NULL DEFAULT '0',
  `dmg_type5` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `armor` smallint(5) unsigned NOT NULL DEFAULT '0',
  `holy_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `fire_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `nature_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `frost_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `shadow_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `arcane_res` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `delay` smallint(5) unsigned NOT NULL DEFAULT '1000',
  `ammo_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RangedModRange` float NOT NULL DEFAULT '0',
  `spellid_1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spelltrigger_1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellcharges_1` tinyint(4) NOT NULL DEFAULT '0',
  `spellppmRate_1` float NOT NULL DEFAULT '0',
  `spellcooldown_1` int(11) NOT NULL DEFAULT '-1',
  `spellcategory_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown_1` int(11) NOT NULL DEFAULT '-1',
  `spellid_2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spelltrigger_2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellcharges_2` tinyint(4) NOT NULL DEFAULT '0',
  `spellppmRate_2` float NOT NULL DEFAULT '0',
  `spellcooldown_2` int(11) NOT NULL DEFAULT '-1',
  `spellcategory_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown_2` int(11) NOT NULL DEFAULT '-1',
  `spellid_3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spelltrigger_3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellcharges_3` tinyint(4) NOT NULL DEFAULT '0',
  `spellppmRate_3` float NOT NULL DEFAULT '0',
  `spellcooldown_3` int(11) NOT NULL DEFAULT '-1',
  `spellcategory_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown_3` int(11) NOT NULL DEFAULT '-1',
  `spellid_4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spelltrigger_4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellcharges_4` tinyint(4) NOT NULL DEFAULT '0',
  `spellppmRate_4` float NOT NULL DEFAULT '0',
  `spellcooldown_4` int(11) NOT NULL DEFAULT '-1',
  `spellcategory_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown_4` int(11) NOT NULL DEFAULT '-1',
  `spellid_5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spelltrigger_5` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellcharges_5` tinyint(4) NOT NULL DEFAULT '0',
  `spellppmRate_5` float NOT NULL DEFAULT '0',
  `spellcooldown_5` int(11) NOT NULL DEFAULT '-1',
  `spellcategory_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown_5` int(11) NOT NULL DEFAULT '-1',
  `bonding` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL DEFAULT '',
  `PageText` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `LanguageID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `PageMaterial` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `startquest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `lockid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Material` tinyint(4) NOT NULL DEFAULT '0',
  `sheath` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RandomProperty` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `block` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemset` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `MaxDurability` smallint(5) unsigned NOT NULL DEFAULT '0',
  `area` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Map` smallint(6) NOT NULL DEFAULT '0',
  `BagFamily` mediumint(9) NOT NULL DEFAULT '0',
  `ScriptName` varchar(64) NOT NULL DEFAULT '',
  `DisenchantID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `FoodType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `minMoneyLoot` int(10) unsigned NOT NULL DEFAULT '0',
  `maxMoneyLoot` int(10) unsigned NOT NULL DEFAULT '0',
  `Duration` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Duration in seconds',
  `ExtraFlags` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`),
  KEY `items_index` (`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item System';

--
-- Dumping data for table `item_template`
--

LOCK TABLES `item_template` WRITE;
/*!40000 ALTER TABLE `item_template` DISABLE KEYS */;
INSERT INTO `item_template` VALUES
(25,2,7,'Worn Shortsword',1542,1,0,1,35,7,21,32767,511,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,1,0,0,0,1,3,0,0,0,20,0,0,0,'',0,0,0,0,0,0),
(35,2,10,'Bent Staff',472,1,0,1,47,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(36,2,4,'Worn Mace',5194,1,0,1,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,3,0,0,0,20,0,0,0,'',0,0,0,0,0,0),
(37,2,0,'Worn Axe',14029,1,0,1,38,7,21,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,20,0,0,0,'',0,0,0,0,0,0),
(38,4,0,'Recruit\'s Shirt',9891,1,0,1,1,1,4,-1,-1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(39,4,1,'Recruit\'s Pants',9892,0,0,1,5,1,7,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(40,4,0,'Recruit\'s Boots',10141,1,0,1,4,1,8,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(43,4,0,'Squire\'s Boots',9938,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(44,4,1,'Squire\'s Pants',9937,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(45,4,0,'Squire\'s Shirt',3265,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(47,4,0,'Footpad\'s Shoes',9915,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(48,4,1,'Footpad\'s Pants',9913,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(49,4,0,'Footpad\'s Shirt',9906,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(51,4,0,'Neophyte\'s Boots',9946,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(52,4,1,'Neophyte\'s Pants',9945,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(53,4,0,'Neophyte\'s Shirt',9944,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(55,4,0,'Apprentice\'s Boots',9929,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(56,4,1,'Apprentice\'s Robe',12647,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(57,4,1,'Acolyte\'s Robe',12645,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(59,4,0,'Acolyte\'s Shoes',3261,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(117,0,0,'Tough Jerky',2473,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,-1,0,-1,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(120,4,1,'Thug Pants',10006,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(121,4,0,'Thug Boots',10008,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(127,4,0,'Trapper\'s Shirt',9996,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(139,4,1,'Brawler\'s Pants',9988,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(140,4,0,'Brawler\'s Boots',9992,1,0,1,4,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(147,4,1,'Rugged Trapper\'s Pants',9975,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(148,4,0,'Rugged Trapper\'s Shirt',9976,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(153,4,2,'Primitive Kilt',10050,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,0,0,'',0,0,0,0,0,0),
(154,4,0,'Primitive Mantle',10058,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(159,0,0,'Refreshing Spring Water',18084,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,430,0,-1,0,-1,59,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(1395,4,1,'Apprentice\'s Pants',9924,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(1396,4,1,'Acolyte\'s Pants',3260,0,0,1,4,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(2070,0,0,'Darnassian Bleu',6353,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,-1,0,-1,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(2092,2,15,'Worn Dagger',6442,1,0,1,35,7,13,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1600,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,3,0,0,0,16,0,0,0,'',0,0,0,0,0,0),
(2101,1,2,'Light Quiver',21328,1,0,1,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(2102,1,3,'Small Ammo Pouch',1816,1,0,1,4,1,18,2047,255,1,1,0,0,0,0,0,0,0,0,1,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,14824,1,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(2105,4,0,'Thug Shirt',10005,1,0,1,5,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(2361,2,5,'Battleworn Hammer',8690,1,0,1,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,1,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(2362,4,6,'Worn Wooden Shield',18730,0,0,1,7,1,14,32767,511,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,4,0,1,0,20,0,0,0,'',0,0,0,0,0,0),
(2504,2,2,'Worn Shortbow',8106,1,0,1,29,5,15,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,2,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,20,0,0,0,'',0,0,0,0,0,0),
(2508,2,3,'Old Blunderbuss',6606,1,0,1,27,5,26,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2300,3,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,20,0,0,0,'',0,0,0,0,0,0),
(2512,6,2,'Rough Arrow',5996,1,0,1,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0),
(2516,6,3,'Light Shot',5998,1,0,1,10,0,24,2047,255,5,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,0,0,0,0,0,0,0,2,'',0,0,0,0,0,0),
(2947,2,16,'Small Throwing Knife',16754,1,0,1,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(3661,2,10,'Handcrafted Staff',18530,1,0,1,45,9,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,2,2,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(4536,0,0,'Shiny Red Apple',6410,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,-1,0,-1,11,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(4540,0,0,'Tough Hunk of Bread',6399,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,-1,0,-1,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(4604,0,0,'Forest Mushroom Cap',15852,1,0,6,25,1,0,2047,255,5,1,0,0,0,0,0,0,0,0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,433,0,-1,0,-1,11,1000,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6096,4,0,'Apprentice\'s Shirt',2163,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6097,4,0,'Acolyte\'s Shirt',2470,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6098,4,1,'Neophyte\'s Robe',12679,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6116,4,1,'Apprentice\'s Robe',12648,0,0,1,4,1,20,-1,-1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,-1,0,0,0,0,-1,0,-1,0,0,0,0,-1,0,-1,0,0,0,0,-1,0,-1,0,0,0,0,-1,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6119,4,1,'Neophyte\'s Robe',12681,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6123,4,1,'Novice\'s Robe',12683,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6124,4,1,'Novice\'s Pants',9987,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(6125,4,0,'Brawler\'s Harness',9995,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6126,4,1,'Trapper\'s Pants',10002,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(6127,4,0,'Trapper\'s Boots',10003,1,0,1,5,1,8,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6129,4,1,'Acolyte\'s Robe',12646,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6134,4,0,'Primitive Mantle',10108,1,0,1,1,1,4,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(6135,4,2,'Primitive Kilt',10109,0,0,1,5,1,7,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,8,0,0,0,0,30,0,0,0,'',0,0,0,0,0,0),
(6139,4,1,'Novice\'s Robe',12684,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6140,4,1,'Apprentice\'s Robe',12649,0,0,1,4,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6144,4,1,'Neophyte\'s Robe',12680,0,0,1,5,1,20,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,7,0,0,0,0,35,0,0,0,'',0,0,0,0,0,0),
(6948,15,0,'Hearthstone',6418,1,64,1,0,0,0,32767,511,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8690,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(12282,2,1,'Worn Battleaxe',22291,1,0,1,43,8,17,2047,255,2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2900,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,1,0,0,0,25,0,0,0,'',0,0,0,0,0,0),
(14646,12,0,'Northshire Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5805,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(14647,12,0,'Coldridge Valley Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5841,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(14648,12,0,'Shadowglen Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5842,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(14649,12,0,'Valley of Trials Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5843,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(14650,12,0,'Camp Narache Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5844,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(14651,12,0,'Deathknell Gift Voucher',18499,1,0,1,0,0,0,2047,255,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1000,0,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,'',0,0,0,5847,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0),
(25861,2,16,'Crude Throwing Axe',20777,1,0,1,15,0,25,2047,255,3,1,0,0,0,0,0,0,0,0,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2000,4,0,0,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,0,1,0,0,0,0,0,0,0,0,'',0,0,0,0,0,0);
/*!40000 ALTER TABLE `item_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_areatrigger_teleport`
--

DROP TABLE IF EXISTS `locales_areatrigger_teleport`;
CREATE TABLE `locales_areatrigger_teleport` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Areatrigger Teleport',
   `Text_loc1` LONGTEXT COMMENT 'Text of the status_failed locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the status_failed locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the status_failed locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the status_failed locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the status_failed locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the status_failed locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the status_failed locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the status_failed locale 8',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Areatrigger System';

--
-- Dumping data for table `locales_areatrigger_teleport`
--

LOCK TABLES `locales_areatrigger_teleport` WRITE;
/*!40000 ALTER TABLE `locales_areatrigger_teleport` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_areatrigger_teleport` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_creature`
--

DROP TABLE IF EXISTS `locales_creature`;
CREATE TABLE `locales_creature` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name_loc1` varchar(100) NOT NULL DEFAULT '',
  `name_loc2` varchar(100) NOT NULL DEFAULT '',
  `name_loc3` varchar(100) NOT NULL DEFAULT '',
  `name_loc4` varchar(100) NOT NULL DEFAULT '',
  `name_loc5` varchar(100) NOT NULL DEFAULT '',
  `name_loc6` varchar(100) NOT NULL DEFAULT '',
  `name_loc7` varchar(100) NOT NULL DEFAULT '',
  `name_loc8` varchar(100) NOT NULL DEFAULT '',
  `subname_loc1` varchar(100) DEFAULT NULL,
  `subname_loc2` varchar(100) DEFAULT NULL,
  `subname_loc3` varchar(100) DEFAULT NULL,
  `subname_loc4` varchar(100) DEFAULT NULL,
  `subname_loc5` varchar(100) DEFAULT NULL,
  `subname_loc6` varchar(100) DEFAULT NULL,
  `subname_loc7` varchar(100) DEFAULT NULL,
  `subname_loc8` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_creature`
--

LOCK TABLES `locales_creature` WRITE;
/*!40000 ALTER TABLE `locales_creature` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_creature` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_gameobject`
--

DROP TABLE IF EXISTS `locales_gameobject`;
CREATE TABLE `locales_gameobject` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name_loc1` varchar(100) NOT NULL DEFAULT '',
  `name_loc2` varchar(100) NOT NULL DEFAULT '',
  `name_loc3` varchar(100) NOT NULL DEFAULT '',
  `name_loc4` varchar(100) NOT NULL DEFAULT '',
  `name_loc5` varchar(100) NOT NULL DEFAULT '',
  `name_loc6` varchar(100) NOT NULL DEFAULT '',
  `name_loc7` varchar(100) NOT NULL DEFAULT '',
  `name_loc8` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_gameobject`
--

LOCK TABLES `locales_gameobject` WRITE;
/*!40000 ALTER TABLE `locales_gameobject` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_gameobject` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_gossip_menu_option`
--

DROP TABLE IF EXISTS `locales_gossip_menu_option`;
CREATE TABLE `locales_gossip_menu_option` (
  `menu_id` smallint(6) unsigned NOT NULL DEFAULT '0',
  `id` smallint(6) unsigned NOT NULL DEFAULT '0',
  `option_text_loc1` text,
  `option_text_loc2` text,
  `option_text_loc3` text,
  `option_text_loc4` text,
  `option_text_loc5` text,
  `option_text_loc6` text,
  `option_text_loc7` text,
  `option_text_loc8` text,
  `box_text_loc1` text,
  `box_text_loc2` text,
  `box_text_loc3` text,
  `box_text_loc4` text,
  `box_text_loc5` text,
  `box_text_loc6` text,
  `box_text_loc7` text,
  `box_text_loc8` text,
  PRIMARY KEY (`menu_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_gossip_menu_option`
--

LOCK TABLES `locales_gossip_menu_option` WRITE;
/*!40000 ALTER TABLE `locales_gossip_menu_option` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_gossip_menu_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_item`
--

DROP TABLE IF EXISTS `locales_item`;
CREATE TABLE `locales_item` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name_loc1` varchar(100) NOT NULL DEFAULT '',
  `name_loc2` varchar(100) NOT NULL DEFAULT '',
  `name_loc3` varchar(100) NOT NULL DEFAULT '',
  `name_loc4` varchar(100) NOT NULL DEFAULT '',
  `name_loc5` varchar(100) NOT NULL DEFAULT '',
  `name_loc6` varchar(100) NOT NULL DEFAULT '',
  `name_loc7` varchar(100) NOT NULL DEFAULT '',
  `name_loc8` varchar(100) NOT NULL DEFAULT '',
  `description_loc1` varchar(255) DEFAULT NULL,
  `description_loc2` varchar(255) DEFAULT NULL,
  `description_loc3` varchar(255) DEFAULT NULL,
  `description_loc4` varchar(255) DEFAULT NULL,
  `description_loc5` varchar(255) DEFAULT NULL,
  `description_loc6` varchar(255) DEFAULT NULL,
  `description_loc7` varchar(255) DEFAULT NULL,
  `description_loc8` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_item`
--

LOCK TABLES `locales_item` WRITE;
/*!40000 ALTER TABLE `locales_item` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_item` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_npc_text`
--

DROP TABLE IF EXISTS `locales_npc_text`;
CREATE TABLE `locales_npc_text` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Text0_0_loc1` longtext,
  `Text0_0_loc2` longtext,
  `Text0_0_loc3` longtext,
  `Text0_0_loc4` longtext,
  `Text0_0_loc5` longtext,
  `Text0_0_loc6` longtext,
  `Text0_0_loc7` longtext,
  `Text0_0_loc8` longtext,
  `Text0_1_loc1` longtext,
  `Text0_1_loc2` longtext,
  `Text0_1_loc3` longtext,
  `Text0_1_loc4` longtext,
  `Text0_1_loc5` longtext,
  `Text0_1_loc6` longtext,
  `Text0_1_loc7` longtext,
  `Text0_1_loc8` longtext,
  `Text1_0_loc1` longtext,
  `Text1_0_loc2` longtext,
  `Text1_0_loc3` longtext,
  `Text1_0_loc4` longtext,
  `Text1_0_loc5` longtext,
  `Text1_0_loc6` longtext,
  `Text1_0_loc7` longtext,
  `Text1_0_loc8` longtext,
  `Text1_1_loc1` longtext,
  `Text1_1_loc2` longtext,
  `Text1_1_loc3` longtext,
  `Text1_1_loc4` longtext,
  `Text1_1_loc5` longtext,
  `Text1_1_loc6` longtext,
  `Text1_1_loc7` longtext,
  `Text1_1_loc8` longtext,
  `Text2_0_loc1` longtext,
  `Text2_0_loc2` longtext,
  `Text2_0_loc3` longtext,
  `Text2_0_loc4` longtext,
  `Text2_0_loc5` longtext,
  `Text2_0_loc6` longtext,
  `Text2_0_loc7` longtext,
  `Text2_0_loc8` longtext,
  `Text2_1_loc1` longtext,
  `Text2_1_loc2` longtext,
  `Text2_1_loc3` longtext,
  `Text2_1_loc4` longtext,
  `Text2_1_loc5` longtext,
  `Text2_1_loc6` longtext,
  `Text2_1_loc7` longtext,
  `Text2_1_loc8` longtext,
  `Text3_0_loc1` longtext,
  `Text3_0_loc2` longtext,
  `Text3_0_loc3` longtext,
  `Text3_0_loc4` longtext,
  `Text3_0_loc5` longtext,
  `Text3_0_loc6` longtext,
  `Text3_0_loc7` longtext,
  `Text3_0_loc8` longtext,
  `Text3_1_loc1` longtext,
  `Text3_1_loc2` longtext,
  `Text3_1_loc3` longtext,
  `Text3_1_loc4` longtext,
  `Text3_1_loc5` longtext,
  `Text3_1_loc6` longtext,
  `Text3_1_loc7` longtext,
  `Text3_1_loc8` longtext,
  `Text4_0_loc1` longtext,
  `Text4_0_loc2` longtext,
  `Text4_0_loc3` longtext,
  `Text4_0_loc4` longtext,
  `Text4_0_loc5` longtext,
  `Text4_0_loc6` longtext,
  `Text4_0_loc7` longtext,
  `Text4_0_loc8` longtext,
  `Text4_1_loc1` longtext,
  `Text4_1_loc2` longtext,
  `Text4_1_loc3` longtext,
  `Text4_1_loc4` longtext,
  `Text4_1_loc5` longtext,
  `Text4_1_loc6` longtext,
  `Text4_1_loc7` longtext,
  `Text4_1_loc8` longtext,
  `Text5_0_loc1` longtext,
  `Text5_0_loc2` longtext,
  `Text5_0_loc3` longtext,
  `Text5_0_loc4` longtext,
  `Text5_0_loc5` longtext,
  `Text5_0_loc6` longtext,
  `Text5_0_loc7` longtext,
  `Text5_0_loc8` longtext,
  `Text5_1_loc1` longtext,
  `Text5_1_loc2` longtext,
  `Text5_1_loc3` longtext,
  `Text5_1_loc4` longtext,
  `Text5_1_loc5` longtext,
  `Text5_1_loc6` longtext,
  `Text5_1_loc7` longtext,
  `Text5_1_loc8` longtext,
  `Text6_0_loc1` longtext,
  `Text6_0_loc2` longtext,
  `Text6_0_loc3` longtext,
  `Text6_0_loc4` longtext,
  `Text6_0_loc5` longtext,
  `Text6_0_loc6` longtext,
  `Text6_0_loc7` longtext,
  `Text6_0_loc8` longtext,
  `Text6_1_loc1` longtext,
  `Text6_1_loc2` longtext,
  `Text6_1_loc3` longtext,
  `Text6_1_loc4` longtext,
  `Text6_1_loc5` longtext,
  `Text6_1_loc6` longtext,
  `Text6_1_loc7` longtext,
  `Text6_1_loc8` longtext,
  `Text7_0_loc1` longtext,
  `Text7_0_loc2` longtext,
  `Text7_0_loc3` longtext,
  `Text7_0_loc4` longtext,
  `Text7_0_loc5` longtext,
  `Text7_0_loc6` longtext,
  `Text7_0_loc7` longtext,
  `Text7_0_loc8` longtext,
  `Text7_1_loc1` longtext,
  `Text7_1_loc2` longtext,
  `Text7_1_loc3` longtext,
  `Text7_1_loc4` longtext,
  `Text7_1_loc5` longtext,
  `Text7_1_loc6` longtext,
  `Text7_1_loc7` longtext,
  `Text7_1_loc8` longtext,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_npc_text`
--

LOCK TABLES `locales_npc_text` WRITE;
/*!40000 ALTER TABLE `locales_npc_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_npc_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_page_text`
--

DROP TABLE IF EXISTS `locales_page_text`;
CREATE TABLE `locales_page_text` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Text_loc1` longtext,
  `Text_loc2` longtext,
  `Text_loc3` longtext,
  `Text_loc4` longtext,
  `Text_loc5` longtext,
  `Text_loc6` longtext,
  `Text_loc7` longtext,
  `Text_loc8` longtext,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_page_text`
--

LOCK TABLES `locales_page_text` WRITE;
/*!40000 ALTER TABLE `locales_page_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_page_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_points_of_interest`
--

DROP TABLE IF EXISTS `locales_points_of_interest`;
CREATE TABLE `locales_points_of_interest` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `icon_name_loc1` text,
  `icon_name_loc2` text,
  `icon_name_loc3` text,
  `icon_name_loc4` text,
  `icon_name_loc5` text,
  `icon_name_loc6` text,
  `icon_name_loc7` text,
  `icon_name_loc8` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_points_of_interest`
--

LOCK TABLES `locales_points_of_interest` WRITE;
/*!40000 ALTER TABLE `locales_points_of_interest` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_points_of_interest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_quest`
--

DROP TABLE IF EXISTS `locales_quest`;
CREATE TABLE `locales_quest` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Title_loc1` text,
  `Title_loc2` text,
  `Title_loc3` text,
  `Title_loc4` text,
  `Title_loc5` text,
  `Title_loc6` text,
  `Title_loc7` text,
  `Title_loc8` text,
  `Details_loc1` text,
  `Details_loc2` text,
  `Details_loc3` text,
  `Details_loc4` text,
  `Details_loc5` text,
  `Details_loc6` text,
  `Details_loc7` text,
  `Details_loc8` text,
  `Objectives_loc1` text,
  `Objectives_loc2` text,
  `Objectives_loc3` text,
  `Objectives_loc4` text,
  `Objectives_loc5` text,
  `Objectives_loc6` text,
  `Objectives_loc7` text,
  `Objectives_loc8` text,
  `OfferRewardText_loc1` text,
  `OfferRewardText_loc2` text,
  `OfferRewardText_loc3` text,
  `OfferRewardText_loc4` text,
  `OfferRewardText_loc5` text,
  `OfferRewardText_loc6` text,
  `OfferRewardText_loc7` text,
  `OfferRewardText_loc8` text,
  `RequestItemsText_loc1` text,
  `RequestItemsText_loc2` text,
  `RequestItemsText_loc3` text,
  `RequestItemsText_loc4` text,
  `RequestItemsText_loc5` text,
  `RequestItemsText_loc6` text,
  `RequestItemsText_loc7` text,
  `RequestItemsText_loc8` text,
  `EndText_loc1` text,
  `EndText_loc2` text,
  `EndText_loc3` text,
  `EndText_loc4` text,
  `EndText_loc5` text,
  `EndText_loc6` text,
  `EndText_loc7` text,
  `EndText_loc8` text,
  `ObjectiveText1_loc1` text,
  `ObjectiveText1_loc2` text,
  `ObjectiveText1_loc3` text,
  `ObjectiveText1_loc4` text,
  `ObjectiveText1_loc5` text,
  `ObjectiveText1_loc6` text,
  `ObjectiveText1_loc7` text,
  `ObjectiveText1_loc8` text,
  `ObjectiveText2_loc1` text,
  `ObjectiveText2_loc2` text,
  `ObjectiveText2_loc3` text,
  `ObjectiveText2_loc4` text,
  `ObjectiveText2_loc5` text,
  `ObjectiveText2_loc6` text,
  `ObjectiveText2_loc7` text,
  `ObjectiveText2_loc8` text,
  `ObjectiveText3_loc1` text,
  `ObjectiveText3_loc2` text,
  `ObjectiveText3_loc3` text,
  `ObjectiveText3_loc4` text,
  `ObjectiveText3_loc5` text,
  `ObjectiveText3_loc6` text,
  `ObjectiveText3_loc7` text,
  `ObjectiveText3_loc8` text,
  `ObjectiveText4_loc1` text,
  `ObjectiveText4_loc2` text,
  `ObjectiveText4_loc3` text,
  `ObjectiveText4_loc4` text,
  `ObjectiveText4_loc5` text,
  `ObjectiveText4_loc6` text,
  `ObjectiveText4_loc7` text,
  `ObjectiveText4_loc8` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `locales_quest`
--

LOCK TABLES `locales_quest` WRITE;
/*!40000 ALTER TABLE `locales_quest` DISABLE KEYS */;
/*!40000 ALTER TABLE `locales_quest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `locales_questgiver_greeting`
--

DROP TABLE IF EXISTS `locales_questgiver_greeting`;
CREATE TABLE `locales_questgiver_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Questgiver',
   `Type` INT(11) UNSIGNED NOT NULL COMMENT 'Type of entry',
   `Text_loc1` LONGTEXT COMMENT 'Text of the greeting locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the greeting locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the greeting locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the greeting locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the greeting locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the greeting locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the greeting locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the greeting locale 8',
   PRIMARY KEY(`Entry`,`Type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Quest and Gossip system';

--
-- Table structure for table `locales_trainer_greeting`
--

DROP TABLE IF EXISTS `locales_trainer_greeting`;
CREATE TABLE `locales_trainer_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Trainer',
   `Text_loc1` LONGTEXT COMMENT 'Text of the greeting locale 1',
   `Text_loc2` LONGTEXT COMMENT 'Text of the greeting locale 2',
   `Text_loc3` LONGTEXT COMMENT 'Text of the greeting locale 3',
   `Text_loc4` LONGTEXT COMMENT 'Text of the greeting locale 4',
   `Text_loc5` LONGTEXT COMMENT 'Text of the greeting locale 5',
   `Text_loc6` LONGTEXT COMMENT 'Text of the greeting locale 6',
   `Text_loc7` LONGTEXT COMMENT 'Text of the greeting locale 7',
   `Text_loc8` LONGTEXT COMMENT 'Text of the greeting locale 8',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Trainer system';

-- Table structure for table `mail_loot_template`
--

DROP TABLE IF EXISTS `mail_loot_template`;
CREATE TABLE `mail_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `mail_loot_template`
--

LOCK TABLES `mail_loot_template` WRITE;
/*!40000 ALTER TABLE `mail_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `mail_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mangos_string`
--

DROP TABLE IF EXISTS `mangos_string`;
CREATE TABLE `mangos_string` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `content_default` text NOT NULL,
  `content_loc1` text,
  `content_loc2` text,
  `content_loc3` text,
  `content_loc4` text,
  `content_loc5` text,
  `content_loc6` text,
  `content_loc7` text,
  `content_loc8` text,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `mangos_string`
--

LOCK TABLES `mangos_string` WRITE;
/*!40000 ALTER TABLE `mangos_string` DISABLE KEYS */;
INSERT INTO `mangos_string` VALUES
(1,'You should select a character or a creature.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(2,'You should select a creature.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(3,'|cffff0000[System Message]: %s|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(4,'|cffff0000[Event Message]: %s|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(5,'There is no help for that command',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(6,'There is no such command',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(7,'There is no such subcommand',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(8,'Command %s have subcommands:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(9,'Commands available to you:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(10,'Incorrect syntax.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(11,'Your account level is: %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(12,'Online players: %u (max: %u) Queued players: %u (max: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(13,'Server uptime: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(14,'Player saved.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(15,'All players saved.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(16,'There are the following active GMs on this server:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(17,'There are no GMs currently logged in on this server.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(18,'Cannot do that while flying.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(19,'Cannot do that in Battlegrounds.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(20,'Target is flying you can\'t do that.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(21,'%s is flying command failed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(22,'You are not mounted so you can\'t dismount.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(23,'Cannot do that while fighting.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(24,'You used it recently.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(25,'Password not changed (unknown error)!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(26,'The password was changed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(27,'The old password is wrong',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(28,'Your account is now locked.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(29,'Your account is now unlocked.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(30,', rank ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(31,' [known]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(32,' [learn]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(33,' [passive]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(34,' [talent]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(35,' [active]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(36,' [complete]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(37,' (offline)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(38,'on',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(39,'off',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(40,'You are: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(41,'visible',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(42,'invisible',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(43,'done',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(44,'You',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(45,' <unknown> ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(46,'<error>',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(47,'<non-existing character>',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(48,'UNKNOWN',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(49,'You must be at least level %u to enter.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(50,'You must be at least level %u and have item %s to enter.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(51,'Hello! Ready for some training?',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(52,'Invaid item count (%u) for item %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(53,'Mail can\'t have more %u item stacks',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(54,'The new passwords do not match',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(55,'Your password can\'t be longer than 16 characters (client limit), password not changed!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(56,'Current Message of the day: \r\n%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(57,'Using World DB: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(58,'Using script library',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(59,'Using creature EventAI: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(61,'Username: ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(62,'Password: ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(63,'Accepts whispers',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(64,'Doesn\'t accept whispers',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(66,'No script library loaded',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(67,'|c00FFFFFF|Announce:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(100,'Global notify: ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(101,'Map: %u (%s) Zone: %u (%s) Area: %u (%s) %s\nX: %f Y: %f Z: %f Orientation: %f\ngrid[%u,%u]cell[%u,%u] InstanceID: %u\n ZoneX: %f ZoneY: %f\nGroundZ: %f FloorZ: %f Have height data (Map: %u VMap: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(102,'%s is already being teleported.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(103,'You can summon a player to your instance only if he is in your party with you as leader.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(104,'You cannot go to the player\'s instance because you are in a party now.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(105,'You can go to the player\'s instance while not being in his party only if your GM mode is on.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(106,'You can not go to player %s from instance to instance.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(107,'You can not summon player %s from instance to instance.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(108,'You are summoning %s%s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(109,'You are being summoned by %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(110,'You are teleporting %s%s to %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(111,'You are being teleported by %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(112,'Player (%s) does not exist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(113,'Appearing at %s\'s location.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(114,'%s is appearing to your location.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(115,'Incorrect values.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(116,'No character selected.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(117,'%s is not in a group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(118,'You changed HP of %s to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(119,'%s changed your HP to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(120,'You changed MANA of %s to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(121,'%s changed your MANA to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(122,'You changed ENERGY of %s to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(123,'%s changed your ENERGY to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(124,'Current energy: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(125,'You changed rage of %s to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(126,'%s changed your rage to %i/%i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(127,'You changed level of %s to %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(128,'GUID %i, faction is %i, flags is %i, npcflag is %i, DY flag is %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(129,'Wrong faction: %u (not found in factiontemplate.dbc).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(130,'You changed GUID=%i \'s Faction to %i, flags to %i, npcflag to %i, dyflag to %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(131,'You changed the %s spellmod %u to value %i for spell with family bit %u for %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(132,'%s changed your spellmod %u to value %i for spell with family bit %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(133,'%s has access to all taxi nodes now (until logout).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(134,'%s has no more access to all taxi nodes now (only visited accessible).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(135,'%s has given you access to all taxi nodes (until logout).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(136,'%s has removed access to all taxi nodes (only visited still accessible).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(137,'You set all speeds to %2.2f from normal of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(138,'%s set all your speeds to %2.2f from normal.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(139,'You set the speed to %2.2f from normal of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(140,'%s set your speed to %2.2f from normal.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(141,'You set the swim speed to %2.2f from normal of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(142,'%s set your swim speed to %2.2f from normal.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(143,'You set the backwards run speed to %2.2f from normal of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(144,'%s set your backwards run speed to %2.2f from normal.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(147,'You set the size %2.2f of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(148,'%s set your size to %2.2f.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(149,'There is no such mount.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(150,'You give a mount to %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(151,'%s gave you a mount.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(152,'USER1: %i, ADD: %i, DIF: %i\n',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(153,'You take all copper of %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(154,'%s took you all of your copper.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(155,'You take %i copper from %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(156,'%s took %i copper from you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(157,'You give %i copper to %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(158,'%s gave you %i copper.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(159,'You hear sound %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(160,'USER2: %i, ADD: %i, RESULT: %i\n',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(161,'Removed bit %i in field %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(162,'Set bit %i in field %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(163,'Teleport location table is empty!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(164,'Teleport location not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(165,'Requires search parameter.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(166,'There are no teleport locations matching your request.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(168,'Locations found are:\n %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(169,'Mail sent to %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(170,'You try to hear sound %u but it doesn\'t exist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(171,'You can\'t teleport self to self!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(172,'server console command',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(175,'Liquid level: %f, ground: %f, type flags %u, status: %d.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(176,'Conversion failed: channel \'%s\' has password set',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(177,'Conversion failed: channel \'%s\' is not eligible',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(178,'Conversion succeeded: channel \'%s\' static status is now %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(179,'(Static)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(180,'(Password)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(181,'There are no matching custom channels at the moment',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(182,'Listing up to %u custom channels matching criterea:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(183,'Areatrigger debugging turned on.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(184,'Areatrigger debugging turned off.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(185,'You have reached areatrigger %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(200,'No selection.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(201,'Object GUID is: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(202,'The name was too long by %i characters.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(203,'Error, name can only contain characters A-Z and a-z.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(204,'The subname was too long by %i characters.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(205,'Not yet implemented',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(206,'Item \'%i\' \'%s\' added to list with maxcount \'%i\' and incrtime \'%i\' ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(207,'Item \'%i\' not found in database.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(208,'Item \'%i\' \'%s\' deleted from vendor list',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(209,'Item \'%i\' not found in vendor list.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(210,'Item \'%i\' already in vendor list.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(211,'Spells of %s reset.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(212,'Spells of %s will reset at next login.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(213,'Talents of %s reset.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(214,'Talents of %s will reset at next login.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(215,'Your spells have been reset.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(216,'Your talents have been reset.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(217,'Unknown case \'%s\' for .resetall command. Type full correct case name.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(218,'Spells will reset for all players at login. Strongly recommend re-login!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(219,'Talents will reset for all players at login. Strongly recommend re-login!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(220,'Cannot find waypoint id %u for %s (in path %i, loaded from %s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(221,'Last Waypoint not found for %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(222,'%s has no path or path empty, path-id %i (loaded from %s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(223,'Creature (GUID: %u) No waypoints found - This is a MaNGOS db problem (single float).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(224,'Cannot access %s on map, maybe you are too far away from its spawn location',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(225,'Creature (GUID: %u) not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(226,'You must select a visual waypoint.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(227,'No visual waypoints found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(228,'Could not create visual waypoint with creatureID: %d',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(229,'All visual waypoints removed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(230,'Could not add waypoint %u to %s (pathId %i stored by %s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(231,'No GUID provided.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(232,'No waypoint number provided.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(233,'Argument required for \'%s\'.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(234,'Added Waypoint %u to %s (PathId %i, path stored by %s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(235,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(236,'Waypoint changed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(237,'Waypoint %s modified.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(238,'WP export successfull.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(239,'No waypoints found inside the database.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(240,'File imported.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(241,'Waypoint removed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(242,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(243,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(244,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(245,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(246,'Waypoint %u for %s (from pathId %i, stored by %s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(247,' Waittime: %d',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(248,' Orientation: %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(249,' ScriptId: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(250,'ScriptID set to non-existing id %u, add it to DBScriptsEngine and reload the table.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(251,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(252,'AIScriptName: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(253,'Forced rename for player %s will be requested at next login.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(254,'Forced rename for player %s (GUID #%u) will be requested at next login.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(255,'Waypoint-Creature (GUID: %u) Not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(256,'Could not find NPC...',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(257,'Creature movement type set to \'%s\', waypoints removed (if any).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(258,'Creature movement type set to \'%s\', waypoints were not removed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(259,'Incorrect value, use on or off',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(260,'Value saved.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(261,'Value saved, you may need to rejoin or clean your client cache.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(262,'Areatrigger ID %u not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(263,'Target map or coordinates is invalid (X: %f Y: %f MapId: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(264,'Zone coordinates is invalid (X: %f Y: %f AreaId: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(265,'Zone %u (%s) is part of instanceable map %u (%s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(266,'Nothing found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(267,'Object not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(268,'Creature not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(270,'Creature Removed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(271,'Creature moved.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(272,'Creature (GUID:%u) must be on the same map as player!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(273,'Game Object (GUID: %u) not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(274,'Game Object (GUID: %u) has references in not found owner %s GO list, can\'t be deleted.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(275,'Game Object (GUID: %u) removed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(276,'Game Object |cffffffff|Hgameobject:%d|h[%s]|h|r (GUID: %u) turned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(277,'Game Object |cffffffff|Hgameobject:%d|h[%s]|h|r (GUID: %u) moved',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(278,'You must select a vendor',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(279,'You must send id for item',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(280,'Vendor has too many items (max 128)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(281,'You can\'t kick self, logout instead',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(282,'Player %s kicked.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(284,'Accepting Whisper: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(285,'Accepting Whisper: ON',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(286,'Accepting Whisper: OFF',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(287,'Creature (GUID: %u) not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(288,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(289,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(290,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(291,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(292,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(293,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(294,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(295,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(296,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(297,'Spawn distance changed to: %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(298,'Spawn time changed to: %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(299,'The %s field of %s was set to %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(300,'Your chat has been disabled for %u minutes.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(301,'You have disabled %s\'s chat for %u minutes.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(302,'Player\'s chat is already enabled.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(303,'Your chat has been enabled.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(304,'You have enabled %s\'s chat.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(305,'Faction %s (%u) reputation of %s was set to %5d!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(307,'No faction found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(308,'Faction %i unknown!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(309,'Invalid parameter %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(310,'delta must be between 0 and %d (inclusive)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(311,'%d - |cffffffff|Hfaction:%d|h[%s]|h|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(312,' [visible]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(313,' [at war]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(314,' [peace forced]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(315,' [hidden]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(316,' [invisible forced]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(317,' [inactive]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(318,'Hated',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(319,'Hostile',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(320,'Unfriendly',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(321,'Neutral',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(322,'Friendly',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(323,'Honored',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(324,'Revered',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(325,'Exalted',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(326,'Faction %s (%u) can\'not have reputation.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(327,' [no reputation]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(328,'Characters at account %s (Id: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(329,'  %s (GUID %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(330,'No players found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(331,'Extended item cost %u not exist',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(332,'GM mode is ON',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(333,'GM mode is OFF',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(334,'GM Chat Badge is ON',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(335,'GM Chat Badge is OFF',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(336,'You repair all %s\'s items.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(337,'All your items repaired by %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(338,'You set waterwalk mode %s for %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(339,'Your waterwalk mode %s by %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(340,'%s is now following you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(341,'%s is not following you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(342,'%s is now not following you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(343,'Creature (Entry: %u) cannot be tamed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(344,'You already have pet.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(347,'TaxiNode ID %u not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(348,'Game Object (Entry: %u) have invalid data and can\'t be spawned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(357,'Areatrigger %u not has target coordinates',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(358,'No areatriggers found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(359,'%s|cffffffff|Hareatrigger_target:%u|h[Trigger target %u]|h|r Map %u X:%f Y:%f Z:%f%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(360,'%s[Trigger target %u] Map %u X:%f Y:%f Z:%f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(361,'|cffffffff|Hareatrigger:%u|h[Trigger %u]|h|r Map %u X:%f Y:%f Z:%f%s%s%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(362,'[Trigger %u] Map %u X:%f Y:%f Z:%f%s%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(363,' (Dist %f)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(364,' [Tavern]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(365,' [Quest]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(366,'Explore quest:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(367,'Required level %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(368,'Required Items:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(369,'Required quest:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(373,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(374,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(375,'UNUSED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(376,'%u - |cffffffff|Hpool:%u|h[%s]|h|r AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(377,'No pools found for map \'%s\' (Id:%u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(378,'You can\'t use this command at non-instanceable map \'%s\' (Id:%u). Use .lookup pool command instead.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(379,'You can\'t use this command without args at non-instanceable map \'%s\' (Id:%u).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(380,'%d%s - |cffffffff|Hcreature:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r Chance:%f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(381,'%d%s - [%s] X:%f Y:%f Z:%f MapId:%d Chance:%f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(382,'%d%s - |cffffffff|Hcreature:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(383,'%d%s - [%s] X:%f Y:%f Z:%f MapId:%d %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(384,'%d%s - |cffffffff|Hgameobject:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r Chance:%f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(385,'%d%s - [%s] X:%f Y:%f Z:%f MapId:%d Chance:%f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(386,'%d%s - |cffffffff|Hgameobject:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(387,'%d%s - [%s] X:%f Y:%f Z:%f MapId:%d %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(388,'Creatures with explicitly chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(389,'Creatures with equal chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(390,'Gameobjects with explicitly chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(391,'Gameobjects with equal chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(392,'Pool %u - Mother pool: %u |cffffffff|Hpool:%u|h[%s]|h|r AutoSpawn: %u MaxLimit: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(393,'Pool %u - Mother pool: %u [%s] AutoSpawn: %u MaxLimit: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(394,'Pool %u - Mother pool: none AutoSpawn: %u MaxLimit: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(395,'No pools found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(396,'%u - [%s] AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(397,'Pools with explicitly chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(398,'Pools with equal chance:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(399,'%u - |cffffffff|Hpool:%u|h[%s]|h|r AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u Chance: %f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(400,'|cffff0000[System Message]:|rScripts reloaded',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(401,'You change security level of account %s to %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(402,'%s changed your security level to %i.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(403,'You have low security level for this.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(404,'Creature movement disabled.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(405,'Creature movement enabled.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(406,'Weather can\'t be changed for this zone.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(407,'Weather system disabled at server.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(408,'%s is banned for %s. Reason: %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(409,'%s is banned permanently for %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(410,'%s %s not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(411,'%s unbanned.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(412,'There was an error removing the ban on %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(413,'Account not exist: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(414,'There is no such character.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(415,'There is no such IP in banlist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(416,'Account %s has never been banned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(417,'Ban history for account %s:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(418,'Ban Date: %s Bantime: %s Still active: %s  Reason: %s Set by: %s Manually unbanned: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(419,'Inf.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(420,'Never',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(421,'Yes',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(422,'No',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(423,'IP: %s\nBan Date: %s\nUnban Date: %s\nRemaining: %s\nReason: %s\nSet by: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(424,'There is no matching IPban.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(425,'There is no matching account.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(426,'There is no banned account owning a character matching this part.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(427,'The following IPs match your pattern:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(428,'The following accounts match your query:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(429,'You learned many spells/skills.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(430,'You learned all spells for class.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(431,'You learned all talents for class.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(432,'You learned all languages.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(433,'You learned all craft skills and recipes.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(434,'Could not find \'%s\'',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(435,'Invalid item id: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(436,'No items found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(437,'Invalid gameobject id: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(438,'Found items %u: %u ( inventory %u mail %u auction %u guild %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(439,'Found gameobjects %u: %u ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(440,'Invalid creature id: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(441,'Found creatures %u: %u ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(442,'No area found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(443,'No item sets found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(444,'No skills found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(445,'No spells found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(446,'No quests found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(447,'No creatures found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(448,'No gameobjects found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(449,'Graveyard #%u doesn\'t exist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(450,'Graveyard #%u already linked to zone #%u (current).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(451,'Graveyard #%u linked to zone #%u (current).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(452,'Graveyard #%u can\'t be linked to subzone or not existed zone #%u (internal error).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(454,'No faction in Graveyard with id= #%u , fix your DB',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(455,'invalid team, please fix database',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(456,'any',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(457,'alliance',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(458,'horde',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(459,'Graveyard #%u (faction: %s) is nearest from linked to zone #%u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(460,'Zone #%u doesn\'t have linked graveyards.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(461,'Zone #%u doesn\'t have linked graveyards for faction: %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(462,'Teleport location already exists!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(463,'Teleport location added.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(464,'Teleport location NOT added: database error.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(465,'Teleport location deleted.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(466,'No taxinodes found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(467,'Target unit has %d auras:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(468,'id: %d eff: %d type: %d duration: %d maxduration: %d name: %s%s%s caster: %s stacks: %d',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(469,'Target unit has %d auras of type %d:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(470,'id: %d eff: %d name: %s%s%s caster: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(471,'Quest %u not found.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(472,'Quest %u started from item. For correct work, please, add item to inventory and start quest in normal way: .additem %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(473,'Quest removed.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(474,' [rewarded]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(475,' [complete]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(476,' [active]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(477,'%s\'s Fly Mode %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(478,'Opcode %u sent to %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(479,'Character loaded successfully!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(480,'Failed to load the character!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(481,'Character dumped successfully!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(482,'Character dump failed!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(483,'Spell %u broken and not allowed to cast or learn!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(484,'Skill %u (%s) for player %s set to %u and current maximum set to %u (without permanent (talent) bonuses).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(485,'Player %s must have skill %u (%s) before using this command.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(486,'Invalid skill id (%u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(487,'You learned default GM spells/skills.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(488,'You already know that spell.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(489,'Target(%s) already know that spell.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(490,'%s doesn\'t know that spell.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(491,'You already forgot that spell.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(492,'All spell cooldowns removed for %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(493,'Spell %u cooldown removed for %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(494,'Command : Additem, itemId = %i, amount = %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(495,'Command : Additemset, itemsetId = %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(496,'Removed itemID = %i, amount = %i from %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(497,'Cannot create item \'%i\' (amount: %i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(498,'You need to provide a guild name!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(499,'Player not found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(500,'Player already has a guild!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(501,'Guild not created! (already exists?)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(502,'No items from itemset \'%u\' found.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(503,'The distance is: (3D) %f (2D) %f - (3D, point-to-point) %f yards.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(504,'Item \'%i\' \'%s\' Item Slot %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(505,'Item \'%i\' doesn\'t exist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(506,'Item \'%i\' \'%s\' Added to Slot %i',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(507,'Item save failed!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(508,'%d - owner: %s (guid: %u account: %u ) %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(509,'%d - sender: %s (guid: %u account: %u ) receiver: %s (guid: %u account: %u ) %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(510,'%d - owner: %s (guid: %u account: %u ) %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(511,'Wrong link type!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(512,'%d - |cffffffff|Hitem:%d:0:0:0:0:0:0:0|h[%s]|h|r %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(513,'%d - |cffffffff|Hquest:%d:%d|h[%s]|h|r %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(514,'%d - |cffffffff|Hcreature_entry:%d|h[%s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(515,'%d%s - |cffffffff|Hcreature:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(516,'%d - |cffffffff|Hgameobject_entry:%d|h[%s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(517,'%d%s, Entry %d - |cffffffff|Hgameobject:%d|h[%s X:%f Y:%f Z:%f MapId:%d]SpawnGroup:%u|h|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(518,'%d - |cffffffff|Hitemset:%d|h[%s %s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(519,'|cffffffff|Htele:%s|h[%s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(520,'%d - |cffffffff|Hspell:%d|h[%s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(521,'%d - |cffffffff|Hskill:%d|h[%s %s]|h|r %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(522,'Game Object (Entry: %u) not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(523,'>> Game Object %s (GUID: %u) at %f %f %f. Orientation %f.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(524,'Selected object:\n|cffffffff|Hgameobject:%d|h[%s]|h|r GUID: %u ID: %u\nX: %f Y: %f Z: %f MapId: %u\nOrientation: %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(525,'>> Add Game Object \'%i\' (%s) (GUID: %i) added at \'%f %f %f\'.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(526,'%s (lowguid: %u) movement generators stack:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(527,'   Idle',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(528,'   Random',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(529,'   Waypoint',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(530,'   Animal random',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(531,'   Confused',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(532,'   Targeted to player %s (lowguid %u) distance %f angle %f mode %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(533,'   Targeted to creature %s (lowguid %u) distance %f angle %f mode %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(534,'   Targeted to <NULL>',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(535,'   Home movement to (X:%f Y:%f Z:%f)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(536,'   Home movement used for player?!?',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(537,'   Taxi flight',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1192,'  Effect movement',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(538,'   Unknown movement generator (%u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(539,'Player selected: %s.\nFaction: %u.\nnpcFlags: %u.\nEntry: %u.\nDisplayID: %u (Native: %u).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(540,'Level: %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(541,'Health (base): %u. (max): %u. (current): %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(542,'Unit Flags: %u.\nDynamic Flags: %u.\nFaction Template: %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(543,'Loot: %u Pickpocket: %u Skinning: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(544,'Position: %f %f %f.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(545,'*** Is a vendor!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(546,'*** Is a trainer!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(547,'InstanceID: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(548,'Player%s %s (guid: %u) Account: %s (id: %u) GMLevel: %u Last IP: %s Last login: %s Latency: %ums',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(549,'Played time: %s Level: %u Money: %ug%us%uc',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(550,'Command .pinfo doesn\'t support \'rep\' option for offline players.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(551,'%s has explored all zones now.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(552,'%s has no more explored zones.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(553,'%s has explored all zones for you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(554,'%s has hidden all zones from you.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(557,'%s level up you to (%i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(558,'%s level down you to (%i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(559,'%s reset your level progress.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(560,'The area has been set as explored.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(561,'The area has been set as not explored.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(564,'The value index %u is too big to %s (count: %u).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(565,'Set for %s field:%u to uint32 value:%u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(566,'You set for %s field:%u to uint32 value: %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(567,'Set for %s field:%u to to float value:%f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(568,'You set for %s field:%u to float value: %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(569,'Get %s uint32 value:[FIELD]:%u [VALUE]:%u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(570,'%s has uint32 value:[FIELD]:%u [VALUE]:%u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(571,'Get %s float value:[FIELD]:%u [VALUE]:%f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(572,'%s has float value:[FIELD]:%u [VALUE]:%f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(573,'Showing all areatriggers in %s %s:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(575,'Modify %s uint32 field:%u to sum with:%i = %u (%i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(576,'You modify for %s uint32 field:%u to sum with:%i = %u (%i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(577,'You are now invisible.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(578,'You are now visible.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(579,'Selected player or creature not have victim.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(580,'You learned all default spells for race/class and completed quests rewarded spells.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(581,'Found near gameobjects (distance %f): %u ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(582,'SpawnTime: Full:%s Remain:%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(583,'%d - |cffffffff|Hgameevent:%d|h[%s]|h|r%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(584,'No event found!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(585,'Event not exist!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(586,'Event %u: %s%s\nStart: %s End: %s Occurence: %s Length: %s\nNext state change: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(587,'Event %u already active!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(588,'Event %u not active!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(589,'   Point movement to (X:%f Y:%f Z:%f)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(590,'   Fear movement',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(591,'   Distract movement',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(592,'You have learned all spells in craft: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(593,'Currently Banned Accounts:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(594,'|    Account    |   BanDate    |   UnbanDate  |  Banned By    |   Ban Reason  |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(595,'Currently Banned IPs:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(596,'|      IP       |   BanDate    |   UnbanDate  |  Banned By    |   Ban Reason  |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(597,'Current gamemasters:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(598,'|    Account    |  GM  |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(599,'No gamemasters.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(600,'The Alliance wins!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(601,'The Horde wins!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(602,'The battle for Warsong Gulch begins in 1 minute.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(603,'The battle for Warsong Gulch begins in 30 seconds. Prepare yourselves!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(604,'Let the battle for Warsong Gulch begin!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(605,'$n captured the Horde flag!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(606,'$n captured the Alliance flag!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(607,'The Horde flag was dropped by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(608,'The Alliance Flag was dropped by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(609,'The Alliance Flag was returned to its base by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(610,'The Horde flag was returned to its base by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(611,'The Horde flag was picked up by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(612,'The Alliance Flag was picked up by $n!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(613,'The flags are now placed at their bases.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(614,'The Alliance flag is now placed at its base.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(615,'The Horde flag is now placed at its base.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(650,'Alliance',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(651,'Horde',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(652,'stables',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(653,'blacksmith',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(654,'farm',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(655,'lumber mill',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(656,'mine',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(657,'The %s has taken the %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(658,'$n has defended the %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(659,'$n has assaulted the %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(660,'$n claims the %s! If left unchallenged, the %s will control it in 1 minute!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(661,'The battle for Arathi Basin begins in 1 minute.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(662,'The battle for Arathi Basin begins in 30 seconds. Prepare yourselves!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(663,'The battle for Arathi Basin has begun!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(664,'The Alliance has gathered $1776W resources, and is near victory!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(665,'The Horde has gathered $1777W resources, and is near victory!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(666,'After your recent battle in %s our best attempts to award you a Mark of Honor failed. Enclosed you will find the Mark of Honor we were not able to deliver to you at the time. Thanks for fighting in %s!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(705,'You must wait %s before speaking again.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(706,'This item(s) has problems with equipping/storing to inventory.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(707,'%s does not wish to be disturbed: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(708,'%s is Away from Keyboard: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(709,'Do not Disturb',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(710,'Away from Keyboard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(711,'Queue status for %s (Lvl: %u to %u)\nQueued alliances: %u (Need at least %u more)\nQueued hordes: %u (Need at least %u more)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(712,'|cffff0000[BG Queue Announcer]:|r %s -- [%u-%u] A: %u/%u, H: %u/%u|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(715,'You don\'t meet Battleground level requirements',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(717,'|cffff0000[BG Queue Announcer]:|r %s -- [%u-%u] Started!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(720,'Your group is too large for this battleground. Please regroup to join.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(727,'Your group has an offline member. Please remove him before joining.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(728,'Your group has players from the opposing faction. You can\'t join the battleground as a group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(729,'Your group has players from different battleground brakets. You can\'t join as group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(730,'Someone in your party is already in this battleground queue. (S)he must leave it before joining as group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(731,'Someone in your party is Deserter. You can\'t join as group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(732,'Someone in your party is already in three battleground queues. You cannot join as group.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(733,'You cannot teleport to a battleground map.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(734,'You cannot summon players to a battleground or arena map.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(735,'You must be in GM mode to teleport to a player in a battleground.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(736,'You cannot teleport to a battleground from another battleground. Please leave the current battleground first.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(739,'Battlegrounds are set to 1v0 for debugging.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(740,'Battlegrounds are set to normal playercount.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(750,'Not enough players. This game will close in %u mins.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(751,'Not enough players. This game will close in %u seconds.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(752,'Only the Alliance can use that portal',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(753,'Only the Horde can use that portal',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(759,'%s was destroyed by the %s!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(760,'The %s is under attack! If left unchecked, the %s will destroy it!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(761,'The %s was taken by the %s!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(762,'The %s was taken by the %s!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(763,'The %s was taken by the %s!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(764,'The %s is under attack! If left unchecked, the %s will capture it!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(765,'The %s has taken the %s! Its supplies will now be used for reinforcements!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(766,'Irondeep Mine',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(767,'Coldtooth Mine',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(768,'Stormpike Aid Station',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(769,'Dun Baldar South Bunker',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(770,'Dun Baldar North Bunker',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(771,'Stormpike Graveyard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(772,'Icewing Bunker',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(773,'Stonehearth Graveyard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(774,'Stonehearth Bunker',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(775,'Snowfall Graveyard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(776,'Iceblood Tower',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(777,'Iceblood Graveyard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(778,'Tower Point',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(779,'Frostwolf Graveyard',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(780,'East Frostwolf Tower',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(781,'West Frostwolf Tower',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(782,'Frostwolf Relief Hut',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(784,'1 minute until the battle for Alterac Valley begins.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(785,'30 seconds until the battle for Alterac Valley begins. Prepare yourselves!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(786,'The battle for Alterac Valley has begun!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(787,'The Alliance Team is running out of reinforcements!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(788,'The Horde Team is running out of reinforcements!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(789,'The Frostwolf General is Dead!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(790,'The Stormpike General is Dead!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(801,'You do not have enough gold',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(802,'You do not have enough free slots',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(803,'Your partner does not have enough free bag slots',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(804,'You do not have permission to perform that function',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(805,'Unknown language',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(806,'You don\'t know that language',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(807,'Please provide character name',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(808,'Player %s not found or offline',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(809,'Account for character %s not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(811,'Guild Master',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(812,'Officer',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(813,'Veteran',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(814,'Member',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(815,'Initiate',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1000,'Exiting daemon...',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1001,'Account deleted: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1002,'Account %s NOT deleted (probably sql file format was updated)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1003,'Account %s NOT deleted (unknown error)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1004,'Account created: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1005,'Account name cannot be longer than 16 characters (client limit), account not created!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1006,'Account with this name already exist!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1007,'Account %s NOT created (probably sql file format was updated)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1008,'Account %s NOT created (unknown error)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1009,'Player %s (Guid: %u) Account %s (Id: %u) deleted.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1010,'| ID         |    Account    |       Character      |       IP        | GM | Expansion |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1012,'========================================================================================',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1013,'| %10u |%15s| %20s | %15s |%4d| %9d |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1014,'No online players.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1015,'Can only quit from a Remote Admin console or the quit command was not entered in full (quit).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1016,'| GUID       | Name                 | Account                      | Delete Date         |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1017,'| %10u | %20s | %15s (%10u) | %19s |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1018,'==========================================================================================',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1019,'No characters found.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1020,'Restoring the following characters:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1021,'Deleting the following characters:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1022,'ERROR: You can only assign a new name for a single selected character!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1023,'Character \'%s\' (GUID: %u Account %u) can\'t be restored: account doesn\'t exist!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1024,'Character \'%s\' (GUID: %u Account %u) can\'t be restored: account character list full!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1025,'Character \'%s\' (GUID: %u Account %u) can\'t be restored: name already in use!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1026,'GUID: %u Name: %s Account: %s (%u) Date: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1027,'Log filters state:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1028,'All log filters set to: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1029,'Command can only be called from a Remote Admin console.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1100,'Account %s (Id: %u) has been granted %u expansion rights.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1101,'Message of the day changed to:\r\n%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1102,'Message sent to %s: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1103,'%d - %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1104,'%d - %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1105,'%d - %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1106,'%d - %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1107,'%d - %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1108,'%d - %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1109,'%d - %s %s %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1110,'%d%s - %s X:%f Y:%f Z:%f MapId:%d',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1111,'%d%s - %s X:%f Y:%f Z:%f MapId:%d',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1112,'Failed to open file: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1113,'Account %s (%u) have max amount allowed characters (client limit)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1114,'Dump file have broken data!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1115,'Invalid character name!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1116,'Invalid character guid!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1117,'Character guid %u in use!',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1118,'%d - guild: %s (guid: %u) %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1119,'You must use male or female as gender.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1120,'You change gender of %s to %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1121,'Your gender changed to %s by %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1122,'(%u/%u +perm %u +temp %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1128,'%d - |cffffffff|Htaxinode:%u|h[%s %s]|h|r (Map:%u X:%f Y:%f Z:%f)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1129,'%d - %s %s (Map:%u X:%f Y:%f Z:%f)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1130,'event started %u \"%s\"',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1131,'event stopped %u \"%s\"',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1132,'   Follow player %s (lowguid %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1133,'   Follow creature %s (lowguid %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1134,'   Follow <NULL>',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1135,'List known talents:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1136,'   (Found talents: %u used talent points: %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1137,'%d%s - |cffffffff|Hgameobject:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1138,'=================================================================================',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1139,'| GUID       | Name                 | Race            | Class           | Level |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1140,'| %10u | %20s | %15s | %15s | %5u |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1141,'%u - |cffffffff|Hplayer:%s|h[%s]|h|r %s %s %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1142,'%u - %s (Online:%s IP:%s GM:%u Expansion:%u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1143,'Spawned by event %u (%s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1144,'Despawned by event %u (%s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1145,'Part of pool %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1146,'Part of pool %u, top pool %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1147,'The (top)pool %u is spawned by event %u (%s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1148,'The (top)pool %u is despawned by event %u (%s)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1149,' (Pool %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1150,' (Event %i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1151,' (Pool %u Event %i)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1152,'[usable]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1153,'Get %s bitstr value:[FIELD]:%u [VALUE]:%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1154,'%s has bitstr value:[FIELD]:%u [VALUE]:%s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1155,'Get %s hex value:[FIELD]:%u [VALUE]:%x',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1156,'%s has hex value:[FIELD]:%u [VALUE]:%x',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1157,'Modify %s hex field:%u %s %x = %x (hex)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1158,'You modify for %s hex field:%u %s %x = %x (hex)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1159,'Modify %s float field:%u to sum with:%f = %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1160,'You modify for %s float field:%u to sum with:%f = %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1165,'Spell %u not have auras.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1166,'Scripting library not found or not accessible.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1167,'Scripting library has wrong list functions (outdated?).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1168,'Scripting library reloaded.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1169,'Scripting library build for different mangosd revision.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1171,'Reloaded AHBot configuration.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1172,'Error while trying to reload AHBot configuration.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1173,'==========================================================',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1174,'|--------------------------------------------------------|',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1175,'|            | Alliance |  Horde   | Neutral  |  Total   |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1176,'          Alliance/Horde/Neutral/Total',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1177,'| %-10s | %8u | %8u | %8u | %8u |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1178,'%-10s = %6u / %6u / %6u / %6u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1179,'Count',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1194,'Current State Information: GOState %u, LootState %u. Collision %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1195,'Current State Information: GOState %u, LootState %u. Collision %s, (door %s by default)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1196,'Debug output for taxi flights is now %s.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1197,'[Taxi]: Progress at node: [%u][%u], next: [%u][%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1198,'[Taxi]: Progress at final node: [%u][%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1199,'[Taxi]: Changing route to [%u].',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1200,'You try to view cinemitic %u but it doesn\'t exist.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1202,'Spell %u %s = %f (*1.88 = %f) DB = %f AP = %f',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1203,'direct heal',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1204,'direct damage',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1205,'dot heal',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1206,'dot damage',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1400,'Private ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1401,'Corporal ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1402,'Sergeant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1403,'Master Sergeant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1404,'Sergeant Major ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1405,'Knight ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1406,'Knight-Lieutenant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1407,'Knight-Captain ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1408,'Knight-Champion ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1409,'Lieutenant Commander ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1410,'Commander ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1411,'Marshal ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1412,'Field Marshal ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1413,'Grand Marshal ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1414,'Scout ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1415,'Grunt ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1416,'Sergeant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1417,'Senior Sergeant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1418,'First Sergeant ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1419,'Stone Guard ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1420,'Blood Guard ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1421,'Legionnaire ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1422,'Centurion ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1423,'Champion ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1424,'Lieutenant General ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1425,'General ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1426,'Warlord ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1427,'High Warlord ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1428,'Game Master ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1429,'No Rank ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1430,'Dishonored ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1431,'Exiled ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1432,'Outlaw ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1433,'Pariah ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1434,'Player: %s - %s (Rank %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1435,'Today: [Honorable Kills: |c0000ff00%u|r] [Dishonorable Kills: |c00ff0000%u|r]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1436,'Yesterday: [Kills: |c0000ff00%u|r] [Honor: %u]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1437,'This Week: [Kills: |c0000ff00%u|r] [Honor: %u]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1438,'Last Week: [Kills: |c0000ff00%u|r] [Honor: %u] [Standing: %u]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1439,'Life Time: [Rank Points: |c0000ff00%f|r] [Honorable Kills: |c0000ff00%u|r] [Dishonorable Kills: |c00ff0000%u|r] [Highest Rank %u: %s]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1500,'%u - [%s] AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u Chance: %f %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1501,'%u - |cffffffff|Hpool:%u|h[%s]|h|r AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1502,'%u - [%s] AutoSpawn: %u MaxLimit: %u Creatures: %u GameObjecs: %u Pools %u %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1503,'Cannot add spawn because no free guids for static spawn in reserved guids range. Server restart is required before command can be used. Also look GuidReserveSize.* config options.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1504,'AI-Information for Npc Entry %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1505,'AIName: %s (%s) ScriptName: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1506,'Current phase = %u',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1507,'Combat-Movement is %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1508,'Melee attacking is %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1509,'Pool %u not found (Maximal pool entry is %u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1510,'[%s]: Ticket not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1511,'[%s]: Player not found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1512,'[%s]: Ticket requires a higher account security level',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1513,'[%s]: Ticket has to be assigned to you',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1514,'[%s]: Ticket has not been seen by a GM yet',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1515,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1516,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1517,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1518,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1519,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1520,'Ticket category id %u does not exist',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1521,'No new tickets found',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1522,'No tickets found matching criterea',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1523,'Listing up to %u tickets matching criterea:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1524,'| Quick actions: |c00FFFFFF<Shift+click>|r on ticket id tag to read the ticket',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1525,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1526,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1527,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1528,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1529,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1530,'|========================================================|',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1531,'Player is now online',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1532,'Player is now offline',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1533,'Ticket was updated',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1534,'Ticket was abandoned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1535,'Ticket was escalated',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1536,'[Tickets]: Queue system status: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1537,'[Tickets]: Closed: %u, average time: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1538,'[Tickets]: Open: %u (%u escalated)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1539,'[Tickets]: Assigned to you: %u (%u online)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1540,'[Tickets]: [%s] has been added by %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1541,'[Tickets]: [%s] has been abandoned by the author',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1542,'[Tickets]: [%s] has been moved to category \'%s\' (%u) by %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1543,'[Tickets]: [%s] has been commented on by %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1544,'[Tickets]: [%s] has been assigned to %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1545,'[Tickets]: [%s] has been escalated (L%u) by %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1546,'[Tickets]: [%s] has been de-escalated by the author\'s edits',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1547,'[Tickets]: [%s] has been closed by %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1548,'[Tickets]: [%s] has been successfully surveyed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1549,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1550,'Open',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1551,'New',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1552,'Assigned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1553,'Escalated',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1554,'Closed',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1555,'Abandoned',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1556,'Ticket Response',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1557,'Your ticket has been marked as Resolved.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1558,'Your ticket has been marked as Discarded.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1559,'GM\'s conclusion:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1560,'| %s L|c00FFFFFF%u|r ticket #|c00FFFFFF%u|r in category: |c00FFFFFF%s|r (|c00FFFFFF%u|r)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1561,'| Player: |Hplayer:%s|h|c00FFFFFF<%s>|r|h (GUID |c00FFFFFF%010u|r) [|c00FFFFFF%s|r] |c00FF1A1A%s|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1562,'| Location: |c00FFFFFF%.2f %.2f %.2f|r Map #|c00FFFFFF%u|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1563,'| Submitted: |c00FFFFFF%s|r (|c00FFFFFF%s|r ago)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1564,'| Last updated: |c00FFFFFF%s|r (|c00FFFFFF%s|r ago)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1565,'| Last read: |c00FFFFFF%s|r (|c00FFFFFF%s|r ago)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1566,'| Last answered: |c00FFFFFF%s|r (|c00FFFFFF%s|r ago)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1567,'| Closed at: |c00FFFFFF%s|r (|c00FFFFFF%s|r ago)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1568,'| Last assigned to: |Hplayer:%s|h|c00FFFFFF<%s>|r|h (GUID |c00FFFFFF%010u|r)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1569,'| Message:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1570,'| GM\'s final conclusion:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1571,'| GM notes:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1572,'| >> |c00FF80FF%s|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1573,'| >> |c0000AEEF%s|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1574,'| Quick actions: %s %s %s %s %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1575,'WHISPER',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1576,'RESOLVE',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1577,'DISCARD',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1578,'SORT',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1579,'NOTE',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1580,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1581,'Ticket notifications',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1582,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1583,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1584,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1585,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1586,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1587,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1588,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1589,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1590,'RESERVED',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1600,'|cffffff00Northpass Tower has been taken by the Horde!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1601,'|cffffff00Northpass Tower has been taken by the Alliance!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1602,'|cffffff00Crown Guard Tower has been taken by the Horde!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1603,'|cffffff00Crown Guard Tower has been taken by the Alliance!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1604,'|cffffff00Eastwall Tower has been taken by the Horde!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1605,'|cffffff00Eastwall Tower has been taken by the Alliance!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1606,'|cffffff00The Plaguewood Tower has been taken by the Horde!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1607,'|cffffff00The Plaguewood Tower has been taken by the Alliance!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1635,'|cffffff00The Horde has collected 200 silithyst!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1636,'|cffffff00The Alliance has collected 200 silithyst!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1702,'Player |cffff0000%s|r [GUID: %u] has |cffff0000%f|r threat, taunt state %u and hostile state %u.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1703,'Showing threat for %s [Entry %u]',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `mangos_string` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_gossip`
--

DROP TABLE IF EXISTS `npc_gossip`;
CREATE TABLE `npc_gossip` (
  `npc_guid` int(10) unsigned NOT NULL DEFAULT '0',
  `textid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`npc_guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_gossip`
--

LOCK TABLES `npc_gossip` WRITE;
/*!40000 ALTER TABLE `npc_gossip` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_gossip` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_text`
--

DROP TABLE IF EXISTS `npc_text`;
CREATE TABLE `npc_text` (
  `ID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `text0_0` longtext,
  `text0_1` longtext,
  `lang0` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob0` float NOT NULL DEFAULT '0',
  `em0_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em0_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em0_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em0_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em0_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em0_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text1_0` longtext,
  `text1_1` longtext,
  `lang1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob1` float NOT NULL DEFAULT '0',
  `em1_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em1_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em1_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em1_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em1_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em1_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text2_0` longtext,
  `text2_1` longtext,
  `lang2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob2` float NOT NULL DEFAULT '0',
  `em2_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em2_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em2_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em2_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em2_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em2_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text3_0` longtext,
  `text3_1` longtext,
  `lang3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob3` float NOT NULL DEFAULT '0',
  `em3_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em3_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em3_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em3_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em3_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em3_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text4_0` longtext,
  `text4_1` longtext,
  `lang4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob4` float NOT NULL DEFAULT '0',
  `em4_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em4_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em4_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em4_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em4_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em4_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text5_0` longtext,
  `text5_1` longtext,
  `lang5` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob5` float NOT NULL DEFAULT '0',
  `em5_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em5_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em5_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em5_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em5_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em5_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text6_0` longtext,
  `text6_1` longtext,
  `lang6` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob6` float NOT NULL DEFAULT '0',
  `em6_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em6_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em6_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em6_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em6_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em6_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `text7_0` longtext,
  `text7_1` longtext,
  `lang7` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `prob7` float NOT NULL DEFAULT '0',
  `em7_0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em7_1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em7_2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em7_3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em7_4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `em7_5` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_text`
--

LOCK TABLES `npc_text` WRITE;
/*!40000 ALTER TABLE `npc_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_text` ENABLE KEYS */;
UNLOCK TABLES;

DROP TABLE IF EXISTS `npc_text_broadcast_text`;
CREATE TABLE `npc_text_broadcast_text` (
`Id` MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'Identifier',
`Prob0` float NOT NULL,
`Prob1` float NOT NULL DEFAULT '0',
`Prob2` float NOT NULL DEFAULT '0',
`Prob3` float NOT NULL DEFAULT '0',
`Prob4` float NOT NULL DEFAULT '0',
`Prob5` float NOT NULL DEFAULT '0',
`Prob6` float NOT NULL DEFAULT '0',
`Prob7` float NOT NULL DEFAULT '0',
`BroadcastTextId0` INT(11) NOT NULL,
`BroadcastTextId1` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId2` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId3` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId4` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId5` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId6` INT(11) NOT NULL DEFAULT '0',
`BroadcastTextId7` INT(11) NOT NULL DEFAULT '0',
PRIMARY KEY(`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT 'Broadcast Text npc_text equivalent';

--
-- Table structure for table `npc_trainer`
--

DROP TABLE IF EXISTS `npc_trainer`;
CREATE TABLE `npc_trainer` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spellcost` int(10) unsigned NOT NULL DEFAULT '0',
  `reqskill` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqskillvalue` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ReqAbility1` INT(11) UNSIGNED DEFAULT NULL,
  `ReqAbility2` INT(11) UNSIGNED DEFAULT NULL,
  `ReqAbility3` INT(11) UNSIGNED DEFAULT NULL,
  `condition_id` INT(11) unsigned NOT NULL default '0',
  UNIQUE KEY `entry_spell` (`entry`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_trainer`
--

LOCK TABLES `npc_trainer` WRITE;
/*!40000 ALTER TABLE `npc_trainer` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_trainer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_trainer_template`
--

DROP TABLE IF EXISTS `npc_trainer_template`;
CREATE TABLE `npc_trainer_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spellcost` int(10) unsigned NOT NULL DEFAULT '0',
  `reqskill` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqskillvalue` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ReqAbility1` INT(11) UNSIGNED DEFAULT NULL,
  `ReqAbility2` INT(11) UNSIGNED DEFAULT NULL,
  `ReqAbility3` INT(11) UNSIGNED DEFAULT NULL,
  `condition_id` INT(11) unsigned NOT NULL default '0',
  UNIQUE KEY `entry_spell` (`entry`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `npc_trainer_template`
--

LOCK TABLES `npc_trainer_template` WRITE;
/*!40000 ALTER TABLE `npc_trainer_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_trainer_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_vendor`
--

DROP TABLE IF EXISTS `npc_vendor`;
CREATE TABLE `npc_vendor` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `incrtime` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Npc System';

--
-- Dumping data for table `npc_vendor`
--

LOCK TABLES `npc_vendor` WRITE;
/*!40000 ALTER TABLE `npc_vendor` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_vendor` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_vendor_template`
--

DROP TABLE IF EXISTS `npc_vendor_template`;
CREATE TABLE `npc_vendor_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `incrtime` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Npc System';

--
-- Dumping data for table `npc_vendor_template`
--

LOCK TABLES `npc_vendor_template` WRITE;
/*!40000 ALTER TABLE `npc_vendor_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `npc_vendor_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `page_text`
--

DROP TABLE IF EXISTS `page_text`;
CREATE TABLE `page_text` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `text` longtext NOT NULL,
  `next_page` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item System';

--
-- Dumping data for table `page_text`
--

LOCK TABLES `page_text` WRITE;
/*!40000 ALTER TABLE `page_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `page_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pet_levelstats`
--

DROP TABLE IF EXISTS `pet_levelstats`;
CREATE TABLE `pet_levelstats` (
  `creature_entry` mediumint(8) unsigned NOT NULL,
  `level` tinyint(3) unsigned NOT NULL,
  `hp` smallint(5) unsigned NOT NULL,
  `mana` smallint(5) unsigned NOT NULL,
  `armor` int(10) unsigned NOT NULL DEFAULT '0',
  `str` smallint(5) unsigned NOT NULL,
  `agi` smallint(5) unsigned NOT NULL,
  `sta` smallint(5) unsigned NOT NULL,
  `inte` smallint(5) unsigned NOT NULL,
  `spi` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`creature_entry`,`level`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 PACK_KEYS=0 COMMENT='Stores pet levels stats.';

--
-- Dumping data for table `pet_levelstats`
--

LOCK TABLES `pet_levelstats` WRITE;
/*!40000 ALTER TABLE `pet_levelstats` DISABLE KEYS */;
/*!40000 ALTER TABLE `pet_levelstats` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pet_name_generation`
--

DROP TABLE IF EXISTS `pet_name_generation`;
CREATE TABLE `pet_name_generation` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `word` tinytext NOT NULL,
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `half` tinyint(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pet_name_generation`
--

LOCK TABLES `pet_name_generation` WRITE;
/*!40000 ALTER TABLE `pet_name_generation` DISABLE KEYS */;
INSERT INTO `pet_name_generation` (`word`,`entry`,`half`) VALUES
('Aba',416,0),
('Az',416,0),
('Bel',416,0),
('Biz',416,0),
('Cho',416,0),
('Dag',416,0),
('Gak',416,0),
('Gar',416,0),
('Gel',416,0),
('Gho',416,0),
('Gob',416,0),
('Gra',416,0),
('Jak',416,0),
('Jub',416,0),
('Kar',416,0),
('Kup',416,0),
('Laz',416,0),
('Nal',416,0),
('Nok',416,0),
('Pag',416,0),
('Pig',416,0),
('Pip',416,0),
('Piz',416,0),
('Quz',416,0),
('Rui',416,0),
('Rul',416,0),
('Rup',416,0),
('Tar',416,0),
('Vol',416,0),
('Yaz',416,0),
('Zep',416,0),
('Zig',416,0),
('Zil',416,0),
('Zor',416,0),
('bis',416,1),
('fip',416,1),
('gup',416,1),
('ham',416,1),
('jub',416,1),
('kin',416,1),
('kol',416,1),
('lop',416,1),
('loz',416,1),
('mat',416,1),
('mir',416,1),
('nam',416,1),
('nar',416,1),
('nik',416,1),
('nip',416,1),
('pad',416,1),
('pep',416,1),
('pit',416,1),
('qua',416,1),
('rai',416,1),
('rin',416,1),
('rot',416,1),
('tai',416,1),
('tal',416,1),
('tik',416,1),
('tip',416,1),
('tog',416,1),
('tuk',416,1),
('uri',416,1),
('yal',416,1),
('yap',416,1),
('Bhee',417,0),
('Bruu',417,0),
('Czaa',417,0),
('Droo',417,0),
('Flaa',417,0),
('Fzuu',417,0),
('Ghaa',417,0),
('Gree',417,0),
('Gzaa',417,0),
('Haa',417,0),
('Haad',417,0),
('Haag',417,0),
('Haap',417,0),
('Jhaa',417,0),
('Jhuu',417,0),
('Khaa',417,0),
('Khii',417,0),
('Khuu',417,0),
('Kree',417,0),
('Luu',417,0),
('Maa',417,0),
('Nhee',417,0),
('Phuu',417,0),
('Pryy',417,0),
('Rhuu',417,0),
('Shaa',417,0),
('Sloo',417,0),
('Sruu',417,0),
('Thoo',417,0),
('Traa',417,0),
('Wraa',417,0),
('Zhaa',417,0),
('dhon',417,1),
('dhum',417,1),
('dhun',417,1),
('dom',417,1),
('don',417,1),
('drom',417,1),
('dym',417,1),
('fenn',417,1),
('fum',417,1),
('fun',417,1),
('ghon',417,1),
('ghun',417,1),
('grom',417,1),
('grym',417,1),
('hom',417,1),
('hon',417,1),
('hun',417,1),
('jhom',417,1),
('kun',417,1),
('lum',417,1),
('mmon',417,1),
('mon',417,1),
('myn',417,1),
('nam',417,1),
('nem',417,1),
('nhym',417,1),
('nom',417,1),
('num',417,1),
('phom',417,1),
('roon',417,1),
('rym',417,1),
('shon',417,1),
('thun',417,1),
('tom',417,1),
('zhem',417,1),
('zhum',417,1),
('zun',417,1),
('Bar',1860,0),
('Bel',1860,0),
('Char',1860,0),
('Grak\'',1860,0),
('Graz\'',1860,0),
('Grim',1860,0),
('Hath',1860,0),
('Hel',1860,0),
('Hok',1860,0),
('Huk',1860,0),
('Jhaz',1860,0),
('Jhom',1860,0),
('Juk\'',1860,0),
('Kal\'',1860,0),
('Klath',1860,0),
('Kon',1860,0),
('Krag',1860,0),
('Krak',1860,0),
('Mak',1860,0),
('Mezz',1860,0),
('Orm',1860,0),
('Phan',1860,0),
('Sar',1860,0),
('Tang',1860,0),
('Than',1860,0),
('Thog',1860,0),
('Thok',1860,0),
('Thul',1860,0),
('Zag\'',1860,0),
('Zang',1860,0),
('Zhar\'',1860,0),
('kath',1860,1),
('doc',1860,1),
('dok',1860,1),
('gak',1860,1),
('garth',1860,1),
('gore',1860,1),
('gorg',1860,1),
('grave',1860,1),
('gron',1860,1),
('juk',1860,1),
('krast',1860,1),
('kresh',1860,1),
('krit',1860,1),
('los',1860,1),
('mon',1860,1),
('mos',1860,1),
('moth',1860,1),
('nagma',1860,1),
('nak',1860,1),
('nar',1860,1),
('nos',1860,1),
('nuz',1860,1),
('phog',1860,1),
('rath',1860,1),
('tast',1860,1),
('taz',1860,1),
('thak',1860,1),
('thang',1860,1),
('thyk',1860,1),
('vhug',1860,1),
('zazt',1860,1),
('Ael',1863,0),
('Aez',1863,0),
('Ang',1863,0),
('Ban',1863,0),
('Bet',1863,0),
('Bro',1863,0),
('Bry',1863,0),
('Cat',1863,0),
('Dir',1863,0),
('Dis',1863,0),
('Dom',1863,0),
('Drus',1863,0),
('Fie',1863,0),
('Fier',1863,0),
('Gly',1863,0),
('Hel',1863,0),
('Hes',1863,0),
('Kal',1863,0),
('Lyn',1863,0),
('Mir',1863,0),
('Nim',1863,0),
('Sar',1863,0),
('Sel',1863,0),
('Vil',1863,0),
('Zah',1863,0),
('aith',1863,1),
('anda',1863,1),
('antia',1863,1),
('evere',1863,1),
('lia',1863,1),
('lissa',1863,1),
('neri',1863,1),
('neth',1863,1),
('nia',1863,1),
('nlissa',1863,1),
('nora',1863,1),
('nva',1863,1),
('nys',1863,1),
('ola',1863,1),
('ona',1863,1),
('ora',1863,1),
('rah',1863,1),
('riana',1863,1),
('riel',1863,1),
('rona',1863,1),
('tai',1863,1),
('tevere',1863,1),
('thea',1863,1),
('vina',1863,1),
('wena',1863,1),
('wyn',1863,1),
('xia',1863,1),
('yla',1863,1),
('yssa',1863,1),
('Flaa',17252,0),
('Haa',17252,0),
('Jhuu',17252,0),
('Shaa',17252,0),
('Thoo',17252,0),
('dhun',17252,1),
('ghun',17252,1),
('roon',17252,1),
('thun',17252,1),
('tom',17252,1);
/*!40000 ALTER TABLE `pet_name_generation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `petcreateinfo_spell`
--

DROP TABLE IF EXISTS `petcreateinfo_spell`;
CREATE TABLE `petcreateinfo_spell` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Spell1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Spell2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Spell3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Spell4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Pet Create Spells';

--
-- Dumping data for table `petcreateinfo_spell`
--

LOCK TABLES `petcreateinfo_spell` WRITE;
/*!40000 ALTER TABLE `petcreateinfo_spell` DISABLE KEYS */;
/*!40000 ALTER TABLE `petcreateinfo_spell` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pickpocketing_loot_template`
--

DROP TABLE IF EXISTS `pickpocketing_loot_template`;
CREATE TABLE `pickpocketing_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `pickpocketing_loot_template`
--

LOCK TABLES `pickpocketing_loot_template` WRITE;
/*!40000 ALTER TABLE `pickpocketing_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `pickpocketing_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_classlevelstats`
--

DROP TABLE IF EXISTS `player_classlevelstats`;
CREATE TABLE `player_classlevelstats` (
  `class` tinyint(3) unsigned NOT NULL,
  `level` tinyint(3) unsigned NOT NULL,
  `basehp` smallint(5) unsigned NOT NULL,
  `basemana` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`class`,`level`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 PACK_KEYS=0 COMMENT='Stores levels stats.';

--
-- Dumping data for table `player_classlevelstats`
--

LOCK TABLES `player_classlevelstats` WRITE;
/*!40000 ALTER TABLE `player_classlevelstats` DISABLE KEYS */;
INSERT INTO `player_classlevelstats` VALUES
(1,1,20,0),
(1,2,29,0),
(1,3,38,0),
(1,4,47,0),
(1,5,56,0),
(1,6,65,0),
(1,7,74,0),
(1,8,83,0),
(1,9,92,0),
(1,10,101,0),
(1,11,100,0),
(1,12,109,0),
(1,13,118,0),
(1,14,128,0),
(1,15,139,0),
(1,16,151,0),
(1,17,154,0),
(1,18,168,0),
(1,19,168,0),
(1,20,199,0),
(1,21,206,0),
(1,22,224,0),
(1,23,243,0),
(1,24,253,0),
(1,25,274,0),
(1,26,296,0),
(1,27,309,0),
(1,28,333,0),
(1,29,348,0),
(1,30,374,0),
(1,31,401,0),
(1,32,419,0),
(1,33,448,0),
(1,34,468,0),
(1,35,499,0),
(1,36,521,0),
(1,37,545,0),
(1,38,581,0),
(1,39,609,0),
(1,40,649,0),
(1,41,681,0),
(1,42,715,0),
(1,43,761,0),
(1,44,799,0),
(1,45,839,0),
(1,46,881,0),
(1,47,935,0),
(1,48,981,0),
(1,49,1029,0),
(1,50,1079,0),
(1,51,1131,0),
(1,52,1185,0),
(1,53,1241,0),
(1,54,1299,0),
(1,55,1359,0),
(1,56,1421,0),
(1,57,1485,0),
(1,58,1551,0),
(1,59,1619,0),
(1,60,1689,0),
(2,1,28,59),
(2,2,26,64),
(2,3,34,84),
(2,4,42,90),
(2,5,50,112),
(2,6,58,120),
(2,7,66,129),
(2,8,84,154),
(2,9,92,165),
(2,10,100,192),
(2,11,108,205),
(2,12,116,219),
(2,13,124,249),
(2,14,132,265),
(2,15,131,282),
(2,16,141,315),
(2,17,152,334),
(2,18,164,354),
(2,19,177,390),
(2,20,191,412),
(2,21,206,435),
(2,22,222,459),
(2,23,239,499),
(2,24,247,525),
(2,25,266,552),
(2,26,286,579),
(2,27,307,621),
(2,28,329,648),
(2,29,342,675),
(2,30,366,702),
(2,31,391,729),
(2,32,407,756),
(2,33,434,798),
(2,34,462,825),
(2,35,481,852),
(2,36,511,879),
(2,37,542,906),
(2,38,564,933),
(2,39,597,960),
(2,40,621,987),
(2,41,656,1014),
(2,42,682,1041),
(2,43,719,1068),
(2,44,747,1110),
(2,45,786,1137),
(2,46,816,1164),
(2,47,857,1176),
(2,48,889,1203),
(2,49,922,1230),
(2,50,966,1257),
(2,51,1001,1284),
(2,52,1037,1311),
(2,53,1084,1338),
(2,54,1122,1365),
(2,55,1161,1392),
(2,56,1201,1419),
(2,57,1252,1446),
(2,58,1294,1458),
(2,59,1337,1485),
(2,60,1381,1512),
(3,1,26,63),
(3,2,33,70),
(3,3,40,91),
(3,4,57,98),
(3,5,64,121),
(3,6,71,130),
(3,7,78,155),
(3,8,85,166),
(3,9,92,193),
(3,10,109,206),
(3,11,116,235),
(3,12,123,250),
(3,13,130,266),
(3,14,138,298),
(3,15,147,316),
(3,16,157,350),
(3,17,168,370),
(3,18,180,391),
(3,19,193,428),
(3,20,207,451),
(3,21,222,475),
(3,22,238,515),
(3,23,255,541),
(3,24,273,568),
(3,25,292,611),
(3,26,312,640),
(3,27,333,670),
(3,28,355,715),
(3,29,378,745),
(3,30,402,775),
(3,31,417,805),
(3,32,443,850),
(3,33,470,880),
(3,34,498,910),
(3,35,527,940),
(3,36,547,970),
(3,37,578,1015),
(3,38,610,1045),
(3,39,643,1075),
(3,40,667,1105),
(3,41,702,1135),
(3,42,738,1180),
(3,43,775,1210),
(3,44,803,1240),
(3,45,842,1270),
(3,46,872,1300),
(3,47,913,1330),
(3,48,955,1360),
(3,49,988,1390),
(3,50,1032,1420),
(3,51,1067,1450),
(3,52,1113,1480),
(3,53,1150,1510),
(3,54,1198,1540),
(3,55,1237,1570),
(3,56,1287,1600),
(3,57,1328,1630),
(3,58,1370,1660),
(3,59,1423,1690),
(3,60,1467,1720),
(4,1,25,0),
(4,2,32,0),
(4,3,49,0),
(4,4,56,0),
(4,5,63,0),
(4,6,80,0),
(4,7,87,0),
(4,8,104,0),
(4,9,111,0),
(4,10,118,0),
(4,11,125,0),
(4,12,142,0),
(4,13,149,0),
(4,14,156,0),
(4,15,173,0),
(4,16,181,0),
(4,17,190,0),
(4,18,200,0),
(4,19,221,0),
(4,20,233,0),
(4,21,246,0),
(4,22,260,0),
(4,23,275,0),
(4,24,301,0),
(4,25,318,0),
(4,26,336,0),
(4,27,355,0),
(4,28,375,0),
(4,29,396,0),
(4,30,428,0),
(4,31,451,0),
(4,32,475,0),
(4,33,500,0),
(4,34,526,0),
(4,35,553,0),
(4,36,581,0),
(4,37,610,0),
(4,38,640,0),
(4,39,671,0),
(4,40,703,0),
(4,41,736,0),
(4,42,770,0),
(4,43,805,0),
(4,44,841,0),
(4,45,878,0),
(4,46,916,0),
(4,47,955,0),
(4,48,995,0),
(4,49,1026,0),
(4,50,1068,0),
(4,51,1111,0),
(4,52,1155,0),
(4,53,1200,0),
(4,54,1246,0),
(4,55,1283,0),
(4,56,1331,0),
(4,57,1380,0),
(4,58,1430,0),
(4,59,1471,0),
(4,60,1523,0),
(5,1,31,110),
(5,2,37,165),
(5,3,42,189),
(5,4,57,200),
(5,5,72,197),
(5,6,77,210),
(5,7,92,224),
(5,8,107,239),
(5,9,112,255),
(5,10,127,272),
(5,11,132,275),
(5,12,147,294),
(5,13,162,314),
(5,14,167,320),
(5,15,182,342),
(5,16,187,365),
(5,17,202,389),
(5,18,217,399),
(5,19,222,425),
(5,20,237,437),
(5,21,242,465),
(5,22,258,494),
(5,23,265,509),
(5,24,283,540),
(5,25,292,557),
(5,26,312,590),
(5,27,333,609),
(5,28,345,644),
(5,29,368,665),
(5,30,382,687),
(5,31,407,725),
(5,32,423,749),
(5,33,450,788),
(5,34,468,812),
(5,35,497,836),
(5,36,517,860),
(5,37,538,899),
(5,38,570,923),
(5,39,593,947),
(5,40,627,971),
(5,41,652,1010),
(5,42,688,1034),
(5,43,715,1058),
(5,44,753,1082),
(5,45,782,1106),
(5,46,812,1130),
(5,47,853,1154),
(5,48,885,1178),
(5,49,918,1202),
(5,50,962,1226),
(5,51,997,1250),
(5,52,1043,1274),
(5,53,1080,1298),
(5,54,1118,1322),
(5,55,1167,1331),
(5,56,1207,1355),
(5,57,1248,1379),
(5,58,1290,1403),
(5,59,1343,1412),
(5,60,1387,1436),
(7,1,27,53),
(7,2,34,61),
(7,3,41,68),
(7,4,48,91),
(7,5,65,100),
(7,6,72,110),
(7,7,79,121),
(7,8,86,133),
(7,9,93,161),
(7,10,100,175),
(7,11,107,190),
(7,12,114,206),
(7,13,121,223),
(7,14,128,241),
(7,15,135,260),
(7,16,142,280),
(7,17,150,301),
(7,18,159,323),
(7,19,169,346),
(7,20,180,370),
(7,21,192,395),
(7,22,205,456),
(7,23,219,448),
(7,24,234,476),
(7,25,240,505),
(7,26,257,579),
(7,27,275,566),
(7,28,294,598),
(7,29,314,631),
(7,30,335,718),
(7,31,347,699),
(7,32,370,733),
(7,33,394,767),
(7,34,419,786),
(7,35,435,820),
(7,36,462,854),
(7,37,490,888),
(7,38,509,922),
(7,39,539,941),
(7,40,570,975),
(7,41,592,1009),
(7,42,625,1028),
(7,43,649,1062),
(7,44,684,1096),
(7,45,710,1115),
(7,46,747,1149),
(7,47,775,1183),
(7,48,814,1202),
(7,49,844,1236),
(7,50,885,1255),
(7,51,917,1289),
(7,52,960,1313),
(7,53,994,1342),
(7,54,1029,1376),
(7,55,1075,1395),
(7,56,1112,1414),
(7,57,1150,1448),
(7,58,1199,1467),
(7,59,1239,1501),
(7,60,1280,1520),
(8,1,31,100),
(8,2,37,170),
(8,3,42,181),
(8,4,57,178),
(8,5,72,191),
(8,6,87,205),
(8,7,92,220),
(8,8,107,221),
(8,9,122,238),
(8,10,127,256),
(8,11,142,275),
(8,12,157,280),
(8,13,162,301),
(8,14,177,323),
(8,15,192,331),
(8,16,197,355),
(8,17,212,365),
(8,18,227,391),
(8,19,232,403),
(8,20,247,431),
(8,21,262,445),
(8,22,267,475),
(8,23,282,491),
(8,24,288,523),
(8,25,305,541),
(8,26,323,575),
(8,27,332,595),
(8,28,352,616),
(8,29,363,652),
(8,30,385,673),
(8,31,408,694),
(8,32,422,730),
(8,33,447,751),
(8,34,463,772),
(8,35,490,793),
(8,36,508,814),
(8,37,537,850),
(8,38,567,871),
(8,39,588,892),
(8,40,620,913),
(8,41,643,934),
(8,42,677,955),
(8,43,702,976),
(8,44,738,997),
(8,45,765,1018),
(8,46,803,1039),
(8,47,832,1060),
(8,48,872,1081),
(8,49,903,1102),
(8,50,945,1108),
(8,51,978,1129),
(8,52,1022,1150),
(8,53,1057,1171),
(8,54,1093,1177),
(8,55,1140,1198),
(8,56,1178,1219),
(8,57,1227,1225),
(8,58,1267,1246),
(8,59,1318,1252),
(8,60,1360,1273),
(9,1,23,59),
(9,2,28,98),
(9,3,43,107),
(9,4,48,117),
(9,5,63,128),
(9,6,68,140),
(9,7,83,153),
(9,8,88,167),
(9,9,103,182),
(9,10,108,198),
(9,11,123,200),
(9,12,128,218),
(9,13,143,237),
(9,14,148,257),
(9,15,153,278),
(9,16,168,300),
(9,17,173,308),
(9,18,189,332),
(9,19,196,357),
(9,20,204,383),
(9,21,223,395),
(9,22,233,423),
(9,23,244,452),
(9,24,266,467),
(9,25,279,498),
(9,26,293,530),
(9,27,318,548),
(9,28,334,582),
(9,29,351,602),
(9,30,379,638),
(9,31,398,674),
(9,32,418,695),
(9,33,439,731),
(9,34,471,752),
(9,35,494,788),
(9,36,518,809),
(9,37,543,830),
(9,38,569,866),
(9,39,606,887),
(9,40,634,923),
(9,41,663,944),
(9,42,693,965),
(9,43,724,1001),
(9,44,756,1022),
(9,45,799,1043),
(9,46,823,1064),
(9,47,868,1100),
(9,48,904,1121),
(9,49,941,1142),
(9,50,979,1163),
(9,51,1018,1184),
(9,52,1058,1205),
(9,53,1089,1226),
(9,54,1131,1247),
(9,55,1184,1268),
(9,56,1228,1289),
(9,57,1273,1310),
(9,58,1319,1331),
(9,59,1366,1352),
(9,60,1414,1373),
(11,1,33,17),
(11,2,41,57),
(11,3,58,65),
(11,4,65,74),
(11,5,82,84),
(11,6,89,95),
(11,7,96,107),
(11,8,113,120),
(11,9,120,134),
(11,10,137,149),
(11,11,144,165),
(11,12,151,182),
(11,13,168,200),
(11,14,175,219),
(11,15,182,239),
(11,16,199,260),
(11,17,206,282),
(11,18,214,305),
(11,19,233,329),
(11,20,243,354),
(11,21,254,380),
(11,22,266,392),
(11,23,289,420),
(11,24,303,449),
(11,25,318,479),
(11,26,334,509),
(11,27,361,524),
(11,28,379,554),
(11,29,398,584),
(11,30,418,614),
(11,31,439,629),
(11,32,461,659),
(11,33,494,689),
(11,34,518,704),
(11,35,543,734),
(11,36,569,749),
(11,37,596,779),
(11,38,624,809),
(11,39,653,824),
(11,40,683,854),
(11,41,714,869),
(11,42,746,899),
(11,43,779,914),
(11,44,823,944),
(11,45,858,959),
(11,46,894,989),
(11,47,921,1004),
(11,48,959,1019),
(11,49,998,1049),
(11,50,1038,1064),
(11,51,1079,1079),
(11,52,1121,1109),
(11,53,1164,1124),
(11,54,1208,1139),
(11,55,1253,1154),
(11,56,1299,1169),
(11,57,1346,1199),
(11,58,1384,1214),
(11,59,1433,1229),
(11,60,1483,1244);
/*!40000 ALTER TABLE `player_classlevelstats` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_levelstats`
--

DROP TABLE IF EXISTS `player_levelstats`;
CREATE TABLE `player_levelstats` (
  `race` tinyint(3) unsigned NOT NULL,
  `class` tinyint(3) unsigned NOT NULL,
  `level` tinyint(3) unsigned NOT NULL,
  `str` tinyint(3) unsigned NOT NULL,
  `agi` tinyint(3) unsigned NOT NULL,
  `sta` tinyint(3) unsigned NOT NULL,
  `inte` tinyint(3) unsigned NOT NULL,
  `spi` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`race`,`class`,`level`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 PACK_KEYS=0 COMMENT='Stores levels stats.';

--
-- Dumping data for table `player_levelstats`
--

LOCK TABLES `player_levelstats` WRITE;
/*!40000 ALTER TABLE `player_levelstats` DISABLE KEYS */;
INSERT INTO `player_levelstats` VALUES
(1,1,1,23,20,22,20,21),
(1,1,2,24,21,23,20,21),
(1,1,3,25,21,24,20,22),
(1,1,4,26,22,25,20,22),
(1,1,5,28,23,26,20,22),
(1,1,6,29,24,27,21,22),
(1,1,7,30,24,28,21,23),
(1,1,8,31,25,29,21,23),
(1,1,9,32,26,30,21,23),
(1,1,10,33,26,31,21,24),
(1,1,11,35,27,33,21,24),
(1,1,12,36,28,34,21,24),
(1,1,13,37,29,35,21,25),
(1,1,14,39,30,36,22,25),
(1,1,15,40,30,37,22,25),
(1,1,16,41,31,38,22,26),
(1,1,17,42,32,40,22,26),
(1,1,18,44,33,41,22,26),
(1,1,19,44,33,41,22,26),
(1,1,20,47,35,43,22,27),
(1,1,21,48,35,45,23,27),
(1,1,22,49,36,46,23,28),
(1,1,23,51,37,47,23,28),
(1,1,24,52,38,49,23,29),
(1,1,25,54,39,50,23,29),
(1,1,26,55,40,51,23,29),
(1,1,27,57,41,53,23,30),
(1,1,28,58,42,54,24,30),
(1,1,29,60,43,56,24,31),
(1,1,30,62,44,57,24,31),
(1,1,31,63,45,58,24,31),
(1,1,32,65,46,60,24,32),
(1,1,33,66,47,61,24,32),
(1,1,34,68,48,63,25,33),
(1,1,35,70,49,64,25,33),
(1,1,36,72,50,66,25,34),
(1,1,37,73,51,68,25,34),
(1,1,38,75,52,69,25,34),
(1,1,39,77,53,71,26,35),
(1,1,40,79,54,72,26,35),
(1,1,41,80,56,74,26,36),
(1,1,42,82,57,76,26,36),
(1,1,43,84,58,77,26,37),
(1,1,44,86,59,79,26,37),
(1,1,45,88,60,81,27,38),
(1,1,46,90,61,83,27,38),
(1,1,47,92,63,84,27,39),
(1,1,48,94,64,86,27,39),
(1,1,49,96,65,88,28,40),
(1,1,50,98,66,90,28,40),
(1,1,51,100,68,92,28,42),
(1,1,52,102,69,94,28,42),
(1,1,53,104,70,96,28,43),
(1,1,54,106,72,98,29,44),
(1,1,55,109,73,100,29,44),
(1,1,56,111,74,102,29,45),
(1,1,57,113,76,104,29,45),
(1,1,58,115,77,106,30,46),
(1,1,59,118,79,108,30,46),
(1,1,60,120,80,110,30,47),
(1,2,1,22,20,22,20,22),
(1,2,2,23,21,23,21,23),
(1,2,3,24,21,24,21,23),
(1,2,4,25,22,25,22,24),
(1,2,5,26,22,26,22,25),
(1,2,6,27,23,27,23,25),
(1,2,7,28,23,28,24,26),
(1,2,8,29,24,28,24,26),
(1,2,9,30,24,29,25,27),
(1,2,10,31,25,30,25,28),
(1,2,11,32,25,31,26,29),
(1,2,12,33,26,32,27,29),
(1,2,13,34,27,33,27,30),
(1,2,14,35,27,34,28,31),
(1,2,15,36,28,36,29,31),
(1,2,16,38,28,37,29,32),
(1,2,17,39,29,38,30,33),
(1,2,18,40,30,39,31,34),
(1,2,19,41,30,40,31,34),
(1,2,20,42,31,41,32,35),
(1,2,21,43,32,42,33,36),
(1,2,22,45,32,43,34,37),
(1,2,23,46,33,44,34,38),
(1,2,24,47,34,46,35,38),
(1,2,25,48,34,47,36,39),
(1,2,26,50,35,48,37,40),
(1,2,27,51,36,49,37,42),
(1,2,28,52,36,50,38,43),
(1,2,29,54,37,52,39,44),
(1,2,30,55,38,53,40,44),
(1,2,31,56,39,54,41,45),
(1,2,32,58,39,56,42,46),
(1,2,33,59,40,57,42,47),
(1,2,34,61,41,58,43,48),
(1,2,35,62,42,60,44,49),
(1,2,36,64,43,61,45,50),
(1,2,37,65,43,62,46,51),
(1,2,38,67,44,64,47,52),
(1,2,39,68,45,65,48,53),
(1,2,40,70,46,67,49,54),
(1,2,41,71,47,68,50,55),
(1,2,42,73,47,70,51,56),
(1,2,43,74,48,71,52,57),
(1,2,44,76,49,73,52,58),
(1,2,45,78,50,74,53,59),
(1,2,46,79,51,76,54,60),
(1,2,47,81,52,77,56,61),
(1,2,48,83,53,79,57,63),
(1,2,49,84,54,81,58,65),
(1,2,50,86,55,82,59,66),
(1,2,51,88,56,84,60,67),
(1,2,52,90,57,86,61,68),
(1,2,53,92,58,87,62,69),
(1,2,54,93,59,89,63,70),
(1,2,55,95,60,91,64,72),
(1,2,56,97,61,93,65,73),
(1,2,57,99,62,94,66,74),
(1,2,58,101,63,96,68,75),
(1,2,59,103,64,98,69,77),
(1,2,60,105,65,100,70,78),
(1,4,1,21,23,21,20,20),
(1,4,2,22,24,22,20,21),
(1,4,3,22,25,22,20,22),
(1,4,4,23,27,23,21,22),
(1,4,5,24,28,24,21,22),
(1,4,6,24,29,24,21,23),
(1,4,7,25,31,25,21,23),
(1,4,8,26,32,25,21,23),
(1,4,9,27,33,26,21,24),
(1,4,10,27,35,27,22,24),
(1,4,11,28,36,28,22,25),
(1,4,12,29,37,28,22,25),
(1,4,13,30,39,29,22,25),
(1,4,14,30,40,30,22,26),
(1,4,15,31,42,30,23,26),
(1,4,16,32,43,31,23,27),
(1,4,17,33,44,32,23,27),
(1,4,18,34,46,33,23,27),
(1,4,19,35,48,33,23,28),
(1,4,20,35,49,34,24,28),
(1,4,21,36,51,35,24,29),
(1,4,22,37,52,36,24,29),
(1,4,23,38,54,37,24,30),
(1,4,24,39,55,37,25,30),
(1,4,25,40,57,38,25,31),
(1,4,26,41,59,39,25,31),
(1,4,27,42,60,40,25,31),
(1,4,28,43,62,41,25,32),
(1,4,29,43,64,42,26,32),
(1,4,30,44,66,42,26,33),
(1,4,31,45,67,43,26,33),
(1,4,32,46,69,44,26,34),
(1,4,33,47,71,45,27,34),
(1,4,34,48,73,46,27,35),
(1,4,35,49,75,47,27,35),
(1,4,36,51,77,48,28,36),
(1,4,37,52,78,49,28,37),
(1,4,38,53,80,50,28,37),
(1,4,39,54,82,51,28,38),
(1,4,40,55,84,52,29,38),
(1,4,41,56,86,53,29,39),
(1,4,42,57,88,54,29,39),
(1,4,43,58,90,55,29,40),
(1,4,44,59,93,56,30,40),
(1,4,45,61,95,57,30,42),
(1,4,46,62,97,58,30,43),
(1,4,47,63,99,59,31,43),
(1,4,48,64,101,60,31,44),
(1,4,49,65,103,62,31,45),
(1,4,50,67,106,63,32,45),
(1,4,51,68,108,64,32,46),
(1,4,52,69,110,65,32,46),
(1,4,53,70,113,66,33,47),
(1,4,54,72,115,67,33,48),
(1,4,55,73,117,69,33,48),
(1,4,56,74,120,70,34,49),
(1,4,57,76,122,71,34,50),
(1,4,58,77,125,72,34,51),
(1,4,59,79,127,74,35,51),
(1,4,60,80,130,75,35,52),
(1,5,1,20,20,20,22,24),
(1,5,2,20,20,20,23,25),
(1,5,3,20,20,21,24,26),
(1,5,4,21,21,21,25,28),
(1,5,5,21,21,21,27,29),
(1,5,6,21,21,22,28,30),
(1,5,7,21,21,22,29,31),
(1,5,8,21,22,22,30,32),
(1,5,9,21,22,23,31,34),
(1,5,10,22,22,23,33,35),
(1,5,11,22,22,24,34,36),
(1,5,12,22,23,24,35,38),
(1,5,13,22,23,24,36,39),
(1,5,14,22,23,25,38,40),
(1,5,15,23,23,25,39,43),
(1,5,16,23,24,26,40,44),
(1,5,17,23,24,26,42,45),
(1,5,18,23,24,26,43,47),
(1,5,19,23,25,27,44,48),
(1,5,20,24,25,27,46,50),
(1,5,21,24,25,28,47,51),
(1,5,22,24,25,28,49,53),
(1,5,23,24,26,29,50,54),
(1,5,24,25,26,29,52,56),
(1,5,25,25,26,30,53,57),
(1,5,26,25,27,30,55,59),
(1,5,27,25,27,30,56,61),
(1,5,28,25,27,31,58,63),
(1,5,29,26,28,31,59,65),
(1,5,30,26,28,32,61,67),
(1,5,31,26,28,32,63,68),
(1,5,32,26,29,33,64,70),
(1,5,33,27,29,33,66,72),
(1,5,34,27,29,34,68,73),
(1,5,35,27,30,34,69,75),
(1,5,36,28,30,35,71,77),
(1,5,37,28,30,36,73,79),
(1,5,38,28,31,36,75,81),
(1,5,39,28,31,37,76,84),
(1,5,40,29,31,37,78,85),
(1,5,41,29,32,38,80,87),
(1,5,42,29,32,38,82,89),
(1,5,43,29,33,39,84,91),
(1,5,44,30,33,39,86,93),
(1,5,45,30,33,40,88,95),
(1,5,46,30,34,41,90,97),
(1,5,47,31,34,41,92,99),
(1,5,48,31,35,42,94,102),
(1,5,49,31,35,43,96,105),
(1,5,50,32,35,43,98,107),
(1,5,51,32,36,44,100,109),
(1,5,52,32,36,44,102,111),
(1,5,53,33,37,45,104,113),
(1,5,54,33,37,46,106,116),
(1,5,55,33,38,46,109,118),
(1,5,56,34,38,47,111,120),
(1,5,57,34,39,48,113,123),
(1,5,58,34,39,49,115,126),
(1,5,59,35,40,49,118,129),
(1,5,60,35,40,50,120,131),
(1,8,1,20,20,20,23,22),
(1,8,2,20,20,20,24,24),
(1,8,3,20,20,21,25,25),
(1,8,4,20,21,21,27,26),
(1,8,5,20,21,21,28,28),
(1,8,6,21,21,21,29,29),
(1,8,7,21,21,22,30,30),
(1,8,8,21,21,22,31,31),
(1,8,9,21,21,22,33,32),
(1,8,10,21,22,23,34,34),
(1,8,11,21,22,23,35,35),
(1,8,12,21,22,23,37,36),
(1,8,13,21,22,24,38,37),
(1,8,14,22,22,24,39,39),
(1,8,15,22,23,24,41,40),
(1,8,16,22,23,25,42,42),
(1,8,17,22,23,25,43,44),
(1,8,18,22,23,25,45,45),
(1,8,19,22,23,26,46,46),
(1,8,20,22,24,26,48,48),
(1,8,21,23,24,26,49,49),
(1,8,22,23,24,27,51,51),
(1,8,23,23,24,27,52,52),
(1,8,24,23,25,28,54,54),
(1,8,25,23,25,28,55,55),
(1,8,26,23,25,28,57,57),
(1,8,27,23,25,29,59,58),
(1,8,28,24,25,29,60,60),
(1,8,29,24,26,30,62,61),
(1,8,30,24,26,30,64,64),
(1,8,31,24,26,30,65,66),
(1,8,32,24,26,31,67,67),
(1,8,33,24,27,31,69,69),
(1,8,34,25,27,32,70,71),
(1,8,35,25,27,32,72,72),
(1,8,36,25,28,33,74,74),
(1,8,37,25,28,33,76,76),
(1,8,38,25,28,33,78,78),
(1,8,39,26,28,34,80,79),
(1,8,40,26,29,34,81,81),
(1,8,41,26,29,35,83,84),
(1,8,42,26,29,35,85,86),
(1,8,43,26,29,36,87,88),
(1,8,44,26,30,36,89,90),
(1,8,45,27,30,37,91,92),
(1,8,46,27,30,37,93,94),
(1,8,47,27,31,38,95,96),
(1,8,48,27,31,38,98,98),
(1,8,49,28,31,39,100,100),
(1,8,50,28,32,39,102,102),
(1,8,51,28,32,40,104,105),
(1,8,52,28,32,40,106,107),
(1,8,53,28,33,41,108,109),
(1,8,54,29,33,42,111,111),
(1,8,55,29,33,42,113,114),
(1,8,56,29,34,43,115,116),
(1,8,57,29,34,43,118,118),
(1,8,58,30,34,44,120,120),
(1,8,59,30,35,44,123,123),
(1,8,60,30,35,45,125,126),
(1,9,1,20,20,21,22,22),
(1,9,2,20,20,22,23,24),
(1,9,3,21,21,22,24,25),
(1,9,4,21,21,23,25,26),
(1,9,5,21,21,23,26,27),
(1,9,6,21,22,24,27,28),
(1,9,7,22,22,24,28,30),
(1,9,8,22,22,25,29,31),
(1,9,9,22,23,25,30,32),
(1,9,10,23,23,26,31,33),
(1,9,11,23,24,26,33,34),
(1,9,12,23,24,27,34,35),
(1,9,13,24,24,27,35,37),
(1,9,14,24,25,28,36,38),
(1,9,15,24,25,29,37,39),
(1,9,16,25,26,29,38,40),
(1,9,17,25,26,30,40,43),
(1,9,18,25,26,30,41,44),
(1,9,19,26,27,31,42,45),
(1,9,20,26,27,32,43,47),
(1,9,21,26,28,32,45,48),
(1,9,22,27,28,33,46,49),
(1,9,23,27,29,34,47,51),
(1,9,24,28,29,34,49,52),
(1,9,25,28,30,35,50,54),
(1,9,26,28,30,36,51,55),
(1,9,27,29,30,36,53,56),
(1,9,28,29,31,37,54,58),
(1,9,29,30,31,38,56,59),
(1,9,30,30,32,38,57,61),
(1,9,31,30,32,39,58,64),
(1,9,32,31,33,40,60,65),
(1,9,33,31,33,41,61,67),
(1,9,34,32,34,41,63,68),
(1,9,35,32,34,42,64,70),
(1,9,36,33,35,43,66,72),
(1,9,37,33,36,44,68,73),
(1,9,38,33,36,45,69,75),
(1,9,39,34,37,45,71,77),
(1,9,40,34,37,46,72,78),
(1,9,41,35,38,47,74,80),
(1,9,42,35,38,48,76,82),
(1,9,43,36,39,48,77,86),
(1,9,44,36,39,50,79,86),
(1,9,45,37,40,50,81,88),
(1,9,46,37,41,51,83,90),
(1,9,47,38,41,52,84,92),
(1,9,48,38,42,53,86,94),
(1,9,49,39,43,54,88,96),
(1,9,50,39,43,55,90,98),
(1,9,51,40,44,56,92,100),
(1,9,52,40,44,57,94,102),
(1,9,53,41,45,58,96,105),
(1,9,54,42,46,59,98,107),
(1,9,55,42,46,60,100,109),
(1,9,56,43,47,61,102,111),
(1,9,57,43,48,62,104,113),
(1,9,58,44,49,63,106,116),
(1,9,59,44,49,64,108,118),
(1,9,60,45,50,65,110,120),
(2,1,1,26,17,24,17,23),
(2,1,2,27,18,25,17,23),
(2,1,3,28,18,26,17,24),
(2,1,4,29,19,27,17,24),
(2,1,5,31,20,28,17,24),
(2,1,6,32,21,29,18,24),
(2,1,7,33,21,30,18,25),
(2,1,8,34,22,31,18,25),
(2,1,9,35,23,32,18,25),
(2,1,10,36,23,33,18,26),
(2,1,11,38,24,35,18,26),
(2,1,12,39,25,36,18,26),
(2,1,13,40,26,37,18,27),
(2,1,14,42,27,38,19,27),
(2,1,15,43,27,39,19,27),
(2,1,16,44,28,40,19,28),
(2,1,17,45,29,42,19,28),
(2,1,18,47,30,43,19,28),
(2,1,19,48,31,44,19,29),
(2,1,20,50,32,45,19,29),
(2,1,21,51,32,47,20,29),
(2,1,22,52,33,48,20,30),
(2,1,23,54,34,49,20,30),
(2,1,24,55,35,51,20,31),
(2,1,25,57,36,52,20,31),
(2,1,26,58,37,53,20,31),
(2,1,27,60,38,55,20,32),
(2,1,28,61,39,56,21,32),
(2,1,29,63,40,58,21,33),
(2,1,30,65,41,59,21,33),
(2,1,31,66,42,60,21,33),
(2,1,32,68,43,62,21,34),
(2,1,33,69,44,63,21,34),
(2,1,34,71,45,65,22,35),
(2,1,35,73,46,66,22,35),
(2,1,36,75,47,68,22,36),
(2,1,37,76,48,70,22,36),
(2,1,38,78,49,71,22,36),
(2,1,39,80,50,73,23,37),
(2,1,40,82,51,74,23,37),
(2,1,41,83,53,76,23,38),
(2,1,42,85,54,78,23,38),
(2,1,43,87,55,79,23,39),
(2,1,44,89,56,81,23,39),
(2,1,45,91,57,83,24,40),
(2,1,46,93,58,85,24,40),
(2,1,47,95,60,86,24,41),
(2,1,48,97,61,88,24,41),
(2,1,49,99,62,90,25,42),
(2,1,50,101,63,92,25,42),
(2,1,51,103,65,94,25,43),
(2,1,52,105,66,96,25,43),
(2,1,53,107,67,98,25,44),
(2,1,54,109,69,100,26,45),
(2,1,55,112,70,102,26,45),
(2,1,56,114,71,104,26,46),
(2,1,57,116,73,106,26,46),
(2,1,58,118,74,108,27,47),
(2,1,59,121,76,110,27,47),
(2,1,60,123,77,112,27,48),
(2,3,1,23,20,23,17,24),
(2,3,2,23,21,24,18,25),
(2,3,3,24,22,25,18,25),
(2,3,4,24,24,25,19,26),
(2,3,5,25,25,26,19,26),
(2,3,6,25,26,27,20,27),
(2,3,7,25,27,28,20,27),
(2,3,8,26,28,29,21,28),
(2,3,9,26,30,30,21,29),
(2,3,10,27,31,30,22,29),
(2,3,11,27,32,31,22,30),
(2,3,12,28,34,32,23,31),
(2,3,13,28,35,33,24,31),
(2,3,14,29,36,34,24,32),
(2,3,15,29,38,35,25,32),
(2,3,16,30,39,36,25,33),
(2,3,17,30,40,37,26,34),
(2,3,18,31,42,38,27,35),
(2,3,19,31,43,39,27,35),
(2,3,20,32,45,40,28,36),
(2,3,21,32,46,41,29,37),
(2,3,22,33,48,42,29,37),
(2,3,23,33,49,43,30,38),
(2,3,24,34,51,44,31,39),
(2,3,25,34,52,45,31,40),
(2,3,26,35,54,46,32,40),
(2,3,27,35,56,47,33,41),
(2,3,28,36,57,48,33,42),
(2,3,29,36,59,49,34,43),
(2,3,30,37,61,50,35,43),
(2,3,31,37,62,52,36,44),
(2,3,32,38,64,53,36,45),
(2,3,33,39,66,54,37,46),
(2,3,34,39,67,55,38,47),
(2,3,35,40,69,56,39,48),
(2,3,36,41,71,58,40,49),
(2,3,37,41,73,59,40,49),
(2,3,38,42,75,60,41,50),
(2,3,39,42,77,61,42,51),
(2,3,40,43,78,63,43,52),
(2,3,41,44,80,64,44,53),
(2,3,42,44,82,65,44,54),
(2,3,43,45,84,66,45,55),
(2,3,44,46,86,68,46,56),
(2,3,45,46,88,69,47,57),
(2,3,46,47,90,71,48,58),
(2,3,47,48,92,72,49,59),
(2,3,48,49,95,73,50,60),
(2,3,49,49,97,75,51,61),
(2,3,50,50,99,76,52,62),
(2,3,51,51,101,78,53,63),
(2,3,52,52,103,79,54,64),
(2,3,53,52,105,81,55,65),
(2,3,54,53,108,82,56,66),
(2,3,55,54,110,84,57,67),
(2,3,56,55,112,85,58,68),
(2,3,57,56,115,87,59,70),
(2,3,58,56,117,89,60,71),
(2,3,59,57,120,90,61,72),
(2,3,60,58,122,92,62,73),
(2,4,1,24,20,23,17,23),
(2,4,2,25,21,24,17,23),
(2,4,3,25,22,24,17,24),
(2,4,4,26,24,25,18,24),
(2,4,5,27,25,26,18,24),
(2,4,6,27,26,26,18,25),
(2,4,7,28,28,27,18,25),
(2,4,8,29,29,27,18,25),
(2,4,9,30,30,28,18,26),
(2,4,10,30,32,29,19,26),
(2,4,11,31,33,30,19,27),
(2,4,12,32,34,30,19,27),
(2,4,13,33,36,31,19,27),
(2,4,14,33,37,32,19,28),
(2,4,15,34,39,32,20,28),
(2,4,16,35,40,33,20,29),
(2,4,17,36,41,34,20,29),
(2,4,18,37,43,35,20,29),
(2,4,19,37,43,35,20,29),
(2,4,20,37,43,35,20,29),
(2,4,21,39,48,37,21,31),
(2,4,22,40,49,38,21,31),
(2,4,23,41,51,39,21,32),
(2,4,24,42,52,39,22,32),
(2,4,25,43,54,40,22,33),
(2,4,26,44,56,41,22,33),
(2,4,27,45,57,42,22,33),
(2,4,28,46,59,43,22,34),
(2,4,29,46,61,44,23,34),
(2,4,30,47,63,44,23,35),
(2,4,31,48,64,45,23,35),
(2,4,32,49,66,46,23,36),
(2,4,33,50,68,47,24,36),
(2,4,34,51,70,48,24,37),
(2,4,35,52,72,49,24,37),
(2,4,36,54,74,50,25,38),
(2,4,37,55,75,51,25,39),
(2,4,38,56,77,52,25,39),
(2,4,39,57,79,53,25,40),
(2,4,40,58,81,54,26,40),
(2,4,41,59,83,55,26,41),
(2,4,42,60,85,56,26,41),
(2,4,43,61,87,57,26,42),
(2,4,44,62,90,58,27,42),
(2,4,45,64,92,59,27,43),
(2,4,46,65,94,60,27,44),
(2,4,47,66,96,61,28,44),
(2,4,48,67,98,62,28,45),
(2,4,49,68,100,64,28,46),
(2,4,50,70,103,65,29,46),
(2,4,51,71,105,66,29,47),
(2,4,52,72,107,67,29,47),
(2,4,53,73,110,68,30,48),
(2,4,54,75,112,69,30,49),
(2,4,55,76,114,71,30,49),
(2,4,56,77,117,72,31,50),
(2,4,57,79,119,73,31,51),
(2,4,58,80,122,74,31,52),
(2,4,59,82,124,76,32,52),
(2,4,60,83,127,77,32,53),
(2,7,1,24,17,23,18,25),
(2,7,2,25,17,24,19,26),
(2,7,3,25,18,25,20,27),
(2,7,4,26,18,26,20,28),
(2,7,5,27,19,26,21,29),
(2,7,6,28,19,27,22,30),
(2,7,7,29,19,28,23,31),
(2,7,8,29,20,29,24,31),
(2,7,9,30,20,30,25,32),
(2,7,10,31,21,31,25,33),
(2,7,11,32,21,32,26,34),
(2,7,12,33,22,33,27,35),
(2,7,13,33,22,34,28,36),
(2,7,14,34,23,35,29,37),
(2,7,15,35,23,36,30,39),
(2,7,16,36,24,37,31,40),
(2,7,17,37,24,38,32,41),
(2,7,18,38,25,39,33,42),
(2,7,19,39,25,40,34,43),
(2,7,20,40,26,41,35,44),
(2,7,21,41,26,42,36,45),
(2,7,22,41,27,43,37,46),
(2,7,23,42,27,44,38,47),
(2,7,24,43,28,45,39,49),
(2,7,25,44,28,47,40,50),
(2,7,26,45,29,48,41,51),
(2,7,27,46,29,49,42,52),
(2,7,28,47,30,50,43,53),
(2,7,29,48,30,51,44,55),
(2,7,30,49,31,52,45,56),
(2,7,31,51,31,54,47,57),
(2,7,32,52,32,55,48,59),
(2,7,33,53,33,56,49,60),
(2,7,34,54,33,57,50,61),
(2,7,35,55,34,59,51,63),
(2,7,36,56,35,60,53,64),
(2,7,37,57,35,61,54,65),
(2,7,38,58,36,63,55,67),
(2,7,39,59,36,64,56,68),
(2,7,40,61,37,65,58,70),
(2,7,41,62,38,67,59,71),
(2,7,42,63,38,68,60,73),
(2,7,43,64,39,70,61,74),
(2,7,44,66,40,71,63,76),
(2,7,45,67,40,73,64,77),
(2,7,46,68,41,74,66,79),
(2,7,47,69,42,76,67,80),
(2,7,48,71,43,77,68,82),
(2,7,49,72,43,79,70,84),
(2,7,50,73,44,80,71,85),
(2,7,51,75,45,82,73,87),
(2,7,52,76,46,83,74,89),
(2,7,53,78,46,85,76,90),
(2,7,54,79,47,87,77,92),
(2,7,55,80,48,88,79,94),
(2,7,56,82,49,90,80,96),
(2,7,57,83,50,92,82,97),
(2,7,58,85,50,93,84,99),
(2,7,59,86,51,95,85,101),
(2,7,60,88,52,97,87,103),
(2,9,1,23,17,23,19,25),
(2,9,2,23,17,24,20,26),
(2,9,3,24,18,24,21,27),
(2,9,4,24,18,25,22,28),
(2,9,5,24,18,25,23,29),
(2,9,6,24,19,26,24,30),
(2,9,7,25,19,26,25,32),
(2,9,8,25,19,27,26,33),
(2,9,9,25,20,27,27,34),
(2,9,10,26,20,28,28,35),
(2,9,11,26,21,28,30,36),
(2,9,12,26,21,29,31,37),
(2,9,13,27,21,29,32,39),
(2,9,14,27,22,30,33,40),
(2,9,15,27,22,31,34,41),
(2,9,16,28,23,31,35,42),
(2,9,17,28,23,32,37,44),
(2,9,18,28,23,32,38,45),
(2,9,19,29,24,33,39,46),
(2,9,20,29,24,34,40,48),
(2,9,21,29,25,34,42,49),
(2,9,22,30,25,34,43,50),
(2,9,23,30,26,35,44,52),
(2,9,24,31,26,35,46,53),
(2,9,25,31,27,36,47,55),
(2,9,26,31,27,37,48,56),
(2,9,27,32,27,37,50,57),
(2,9,28,32,28,38,51,59),
(2,9,29,33,28,39,53,60),
(2,9,30,33,29,39,54,62),
(2,9,31,33,29,40,55,64),
(2,9,32,34,30,41,57,65),
(2,9,33,34,30,41,58,68),
(2,9,34,35,31,42,60,68),
(2,9,35,35,31,43,61,70),
(2,9,36,36,32,44,63,71),
(2,9,37,36,33,45,65,73),
(2,9,38,36,33,46,66,75),
(2,9,39,37,34,46,68,77),
(2,9,40,37,34,47,69,78),
(2,9,41,38,35,47,71,80),
(2,9,42,38,35,47,73,83),
(2,9,43,39,36,50,74,84),
(2,9,44,39,36,51,76,85),
(2,9,45,40,37,51,78,87),
(2,9,46,40,38,52,80,89),
(2,9,47,41,38,53,81,91),
(2,9,48,41,39,53,83,93),
(2,9,49,42,40,54,85,96),
(2,9,50,42,40,56,87,97),
(2,9,51,43,41,56,89,99),
(2,9,52,43,41,58,91,101),
(2,9,53,44,42,59,93,103),
(2,9,54,45,43,60,95,105),
(2,9,55,45,43,61,97,106),
(2,9,56,46,44,62,99,109),
(2,9,57,46,45,62,101,110),
(2,9,58,47,46,64,103,114),
(2,9,59,47,46,65,105,116),
(2,9,60,48,47,66,107,118),
(3,1,1,25,16,25,19,19),
(3,1,2,26,17,26,19,19),
(3,1,3,27,17,27,19,20),
(3,1,4,28,18,28,19,20),
(3,1,5,30,19,29,19,20),
(3,1,6,31,20,30,20,20),
(3,1,7,32,20,31,20,21),
(3,1,8,33,21,32,20,21),
(3,1,9,34,22,33,20,21),
(3,1,10,35,22,34,20,22),
(3,1,11,37,23,36,20,22),
(3,1,12,38,24,37,20,22),
(3,1,13,39,25,38,20,23),
(3,1,14,41,26,39,21,23),
(3,1,15,42,26,40,21,23),
(3,1,16,43,27,41,21,24),
(3,1,17,44,28,43,21,24),
(3,1,18,46,29,44,21,24),
(3,1,19,47,30,45,21,25),
(3,1,20,49,31,46,21,25),
(3,1,21,50,31,48,22,25),
(3,1,22,51,32,49,22,26),
(3,1,23,53,33,50,22,26),
(3,1,24,54,34,52,22,27),
(3,1,25,56,35,53,22,27),
(3,1,26,57,36,54,22,27),
(3,1,27,59,37,56,22,28),
(3,1,28,60,38,57,23,28),
(3,1,29,62,39,59,23,29),
(3,1,30,64,40,60,23,29),
(3,1,31,65,41,61,23,29),
(3,1,32,67,42,63,23,30),
(3,1,33,68,43,64,23,30),
(3,1,34,70,44,66,24,31),
(3,1,35,72,45,67,24,31),
(3,1,36,74,46,69,24,32),
(3,1,37,75,47,71,24,32),
(3,1,38,77,48,72,24,32),
(3,1,39,79,49,74,25,33),
(3,1,40,81,50,75,25,33),
(3,1,41,82,52,77,25,34),
(3,1,42,84,53,79,25,34),
(3,1,43,86,54,80,25,35),
(3,1,44,88,55,82,25,35),
(3,1,45,90,56,84,26,36),
(3,1,46,92,57,86,26,36),
(3,1,47,94,59,87,26,37),
(3,1,48,96,60,89,26,37),
(3,1,49,98,61,91,27,38),
(3,1,50,100,62,93,27,38),
(3,1,51,102,64,95,27,39),
(3,1,52,104,65,97,27,39),
(3,1,53,106,66,99,27,40),
(3,1,54,108,68,101,28,41),
(3,1,55,111,69,103,28,41),
(3,1,56,113,70,105,28,42),
(3,1,57,115,72,107,28,42),
(3,1,58,117,73,109,29,43),
(3,1,59,120,75,111,29,43),
(3,1,60,122,76,113,29,44),
(3,2,1,24,16,25,19,20),
(3,2,2,25,17,26,20,21),
(3,2,3,26,17,27,20,21),
(3,2,4,27,18,28,21,22),
(3,2,5,28,18,29,21,23),
(3,2,6,29,19,30,22,23),
(3,2,7,30,19,31,23,24),
(3,2,8,31,20,31,23,24),
(3,2,9,32,20,32,24,25),
(3,2,10,33,21,33,24,26),
(3,2,11,34,21,34,25,27),
(3,2,12,35,22,35,26,27),
(3,2,13,36,23,36,26,28),
(3,2,14,37,23,37,27,29),
(3,2,15,38,24,39,28,29),
(3,2,16,40,24,40,28,30),
(3,2,17,41,25,41,29,31),
(3,2,18,42,26,42,30,32),
(3,2,19,43,26,43,30,32),
(3,2,20,44,27,44,31,33),
(3,2,21,45,28,45,32,34),
(3,2,22,47,28,46,33,35),
(3,2,23,48,29,47,33,36),
(3,2,24,49,30,49,34,36),
(3,2,25,50,30,50,35,37),
(3,2,26,52,31,51,36,38),
(3,2,27,53,32,52,36,39),
(3,2,28,54,32,53,37,40),
(3,2,29,56,33,55,38,41),
(3,2,30,57,34,56,39,41),
(3,2,31,58,35,57,40,42),
(3,2,32,60,35,59,41,43),
(3,2,33,61,36,60,41,44),
(3,2,34,63,37,61,42,45),
(3,2,35,64,38,63,43,46),
(3,2,36,66,39,64,44,47),
(3,2,37,67,39,65,45,48),
(3,2,38,69,40,67,46,49),
(3,2,39,70,41,68,47,50),
(3,2,40,72,42,70,48,51),
(3,2,41,73,43,71,49,52),
(3,2,42,75,43,73,50,53),
(3,2,43,76,44,74,51,54),
(3,2,44,78,45,76,51,55),
(3,2,45,80,46,77,52,56),
(3,2,46,81,47,79,53,57),
(3,2,47,83,48,80,55,58),
(3,2,48,85,49,82,56,59),
(3,2,49,86,50,84,57,61),
(3,2,50,88,51,85,58,62),
(3,2,51,90,52,87,59,63),
(3,2,52,92,53,89,60,64),
(3,2,53,94,54,90,61,65),
(3,2,54,95,55,92,62,66),
(3,2,55,97,56,94,63,68),
(3,2,56,99,57,96,64,69),
(3,2,57,101,58,97,65,70),
(3,2,58,103,59,99,67,71),
(3,2,59,105,60,101,68,73),
(3,2,60,107,61,103,69,74),
(3,3,1,22,19,24,19,20),
(3,3,2,22,20,25,20,21),
(3,3,3,23,21,26,20,21),
(3,3,4,23,23,26,21,22),
(3,3,5,24,24,27,21,22),
(3,3,6,24,25,28,22,23),
(3,3,7,24,26,29,22,23),
(3,3,8,25,27,30,23,24),
(3,3,9,25,29,31,23,25),
(3,3,10,26,30,31,24,25),
(3,3,11,26,31,32,24,26),
(3,3,12,27,33,33,25,27),
(3,3,13,27,34,34,26,27),
(3,3,14,28,35,35,26,28),
(3,3,15,28,37,36,27,28),
(3,3,16,29,38,37,27,29),
(3,3,17,29,39,38,28,30),
(3,3,18,30,41,39,29,31),
(3,3,19,30,42,40,29,31),
(3,3,20,31,44,41,30,32),
(3,3,21,31,45,42,31,33),
(3,3,22,32,47,43,31,33),
(3,3,23,32,48,44,32,34),
(3,3,24,33,50,45,33,35),
(3,3,25,33,51,46,33,36),
(3,3,26,34,53,47,34,36),
(3,3,27,34,55,48,35,37),
(3,3,28,35,56,49,35,38),
(3,3,29,35,58,50,36,39),
(3,3,30,36,60,51,37,39),
(3,3,31,36,61,53,38,40),
(3,3,32,37,63,54,38,41),
(3,3,33,38,65,55,39,42),
(3,3,34,38,66,56,40,43),
(3,3,35,39,68,57,41,44),
(3,3,36,40,70,59,42,45),
(3,3,37,40,72,60,42,45),
(3,3,38,41,74,61,43,46),
(3,3,39,41,76,62,44,47),
(3,3,40,42,77,64,45,48),
(3,3,41,43,79,65,46,49),
(3,3,42,43,81,66,46,50),
(3,3,43,44,83,67,47,51),
(3,3,44,45,85,69,48,52),
(3,3,45,45,87,70,49,53),
(3,3,46,46,89,72,50,54),
(3,3,47,47,91,73,51,55),
(3,3,48,48,94,74,52,56),
(3,3,49,48,96,76,53,57),
(3,3,50,49,98,77,54,58),
(3,3,51,50,100,79,55,59),
(3,3,52,51,102,80,56,60),
(3,3,53,51,104,82,57,61),
(3,3,54,52,107,83,58,62),
(3,3,55,53,109,85,59,63),
(3,3,56,54,111,86,60,64),
(3,3,57,55,114,88,61,66),
(3,3,58,55,116,90,62,67),
(3,3,59,56,119,91,63,68),
(3,3,60,57,121,93,64,69),
(3,4,1,23,19,24,19,19),
(3,4,2,24,20,25,19,19),
(3,4,3,24,21,25,19,20),
(3,4,4,25,23,26,20,20),
(3,4,5,26,24,27,20,20),
(3,4,6,26,25,27,20,21),
(3,4,7,27,27,28,20,21),
(3,4,8,28,28,28,20,21),
(3,4,9,29,29,29,20,22),
(3,4,10,29,31,30,21,22),
(3,4,11,30,32,31,21,23),
(3,4,12,31,33,31,21,23),
(3,4,13,32,35,32,21,23),
(3,4,14,32,36,33,21,24),
(3,4,15,33,38,33,22,24),
(3,4,16,34,39,34,22,25),
(3,4,17,35,40,35,22,25),
(3,4,18,36,42,36,22,25),
(3,4,19,37,44,36,22,26),
(3,4,20,37,45,37,23,26),
(3,4,21,38,47,38,23,27),
(3,4,22,39,48,39,23,27),
(3,4,23,40,50,40,23,28),
(3,4,24,41,51,40,24,28),
(3,4,25,42,53,41,24,29),
(3,4,26,43,55,42,24,29),
(3,4,27,44,56,43,24,29),
(3,4,28,45,58,44,24,30),
(3,4,29,45,60,45,25,30),
(3,4,30,46,62,45,25,31),
(3,4,31,47,63,46,25,31),
(3,4,32,48,65,47,25,32),
(3,4,33,49,67,48,26,32),
(3,4,34,50,69,49,26,33),
(3,4,35,51,71,50,26,33),
(3,4,36,53,73,51,27,34),
(3,4,37,54,74,52,27,35),
(3,4,38,55,76,53,27,35),
(3,4,39,56,78,54,27,36),
(3,4,40,57,80,55,28,36),
(3,4,41,58,82,56,28,37),
(3,4,42,59,84,57,28,37),
(3,4,43,60,86,58,28,38),
(3,4,44,61,89,59,29,38),
(3,4,45,63,91,60,29,39),
(3,4,46,64,93,61,29,40),
(3,4,47,65,95,62,30,40),
(3,4,48,66,97,63,30,41),
(3,4,49,67,99,65,30,42),
(3,4,50,69,102,66,31,42),
(3,4,51,70,104,67,31,43),
(3,4,52,71,106,68,31,43),
(3,4,53,72,109,69,32,44),
(3,4,54,74,111,70,32,45),
(3,4,55,75,113,72,32,45),
(3,4,56,76,116,73,33,46),
(3,4,57,78,118,74,33,47),
(3,4,58,79,121,75,33,48),
(3,4,59,81,123,77,34,48),
(3,4,60,82,126,78,34,49),
(3,5,1,22,16,23,21,22),
(3,5,2,22,16,23,22,23),
(3,5,3,22,16,24,23,24),
(3,5,4,23,17,24,24,26),
(3,5,5,23,17,24,26,27),
(3,5,6,23,17,25,27,28),
(3,5,7,23,17,25,28,29),
(3,5,8,23,18,25,29,30),
(3,5,9,23,18,26,30,32),
(3,5,10,24,18,26,32,33),
(3,5,11,24,18,27,33,34),
(3,5,12,24,19,27,34,36),
(3,5,13,24,19,27,35,37),
(3,5,14,24,19,28,37,38),
(3,5,15,25,19,28,38,40),
(3,5,16,25,20,29,39,41),
(3,5,17,25,20,29,41,42),
(3,5,18,25,20,29,42,44),
(3,5,19,25,21,30,43,45),
(3,5,20,26,21,30,45,47),
(3,5,21,26,21,31,46,48),
(3,5,22,26,21,31,48,50),
(3,5,23,26,22,32,49,51),
(3,5,24,27,22,32,51,53),
(3,5,25,27,22,33,52,54),
(3,5,26,27,23,33,54,56),
(3,5,27,27,23,33,55,58),
(3,5,28,27,23,34,57,59),
(3,5,29,28,24,34,58,61),
(3,5,30,28,24,35,60,63),
(3,5,31,28,24,35,62,64),
(3,5,32,28,25,36,63,66),
(3,5,33,29,25,36,65,68),
(3,5,34,29,25,37,67,69),
(3,5,35,29,26,37,68,71),
(3,5,36,30,26,38,70,73),
(3,5,37,30,26,39,72,75),
(3,5,38,30,27,39,74,77),
(3,5,39,30,27,40,75,79),
(3,5,40,31,27,40,77,80),
(3,5,41,31,28,41,79,82),
(3,5,42,31,28,41,81,84),
(3,5,43,31,29,42,83,86),
(3,5,44,32,29,42,85,88),
(3,5,45,32,29,43,87,90),
(3,5,46,32,30,44,89,92),
(3,5,47,33,30,44,91,94),
(3,5,48,33,31,45,93,97),
(3,5,49,33,31,46,95,99),
(3,5,50,34,31,46,97,101),
(3,5,51,34,32,47,99,103),
(3,5,52,34,32,47,101,105),
(3,5,53,35,33,48,103,107),
(3,5,54,35,33,49,105,110),
(3,5,55,35,34,49,108,112),
(3,5,56,36,34,50,110,114),
(3,5,57,36,35,51,112,117),
(3,5,58,36,35,52,114,119),
(3,5,59,37,36,52,117,122),
(3,5,60,37,36,53,119,124),
(4,1,1,20,25,21,20,20),
(4,1,2,21,26,22,20,20),
(4,1,3,22,26,23,20,21),
(4,1,4,23,27,24,20,21),
(4,1,5,25,28,25,20,21),
(4,1,6,26,29,26,21,21),
(4,1,7,27,29,27,21,22),
(4,1,8,28,30,28,21,22),
(4,1,9,29,31,29,21,22),
(4,1,10,30,31,30,21,23),
(4,1,11,32,32,32,21,23),
(4,1,12,33,33,33,21,23),
(4,1,13,34,34,34,21,24),
(4,1,14,36,35,35,22,24),
(4,1,15,37,35,36,22,24),
(4,1,16,38,36,37,22,25),
(4,1,17,39,37,39,22,25),
(4,1,18,41,38,40,22,25),
(4,1,19,42,39,41,22,26),
(4,1,20,44,40,42,22,26),
(4,1,21,45,40,44,23,26),
(4,1,22,46,41,45,23,27),
(4,1,23,48,42,46,23,27),
(4,1,24,49,43,48,23,28),
(4,1,25,51,44,49,23,28),
(4,1,26,52,45,50,23,28),
(4,1,27,54,46,52,23,29),
(4,1,28,55,47,53,24,29),
(4,1,29,57,48,55,24,30),
(4,1,30,59,49,56,24,30),
(4,1,31,60,50,57,24,30),
(4,1,32,62,51,59,24,31),
(4,1,33,63,52,60,24,31),
(4,1,34,65,53,62,25,32),
(4,1,35,67,54,63,25,32),
(4,1,36,69,55,65,25,33),
(4,1,37,70,56,67,25,33),
(4,1,38,72,57,68,25,33),
(4,1,39,74,58,70,26,34),
(4,1,40,76,59,71,26,34),
(4,1,41,77,61,73,26,35),
(4,1,42,79,62,75,26,35),
(4,1,43,81,63,76,26,36),
(4,1,44,83,64,78,26,36),
(4,1,45,85,65,80,27,37),
(4,1,46,87,66,82,27,37),
(4,1,47,89,68,83,27,38),
(4,1,48,91,69,85,27,38),
(4,1,49,93,70,87,28,39),
(4,1,50,95,71,89,28,39),
(4,1,51,97,73,91,28,40),
(4,1,52,99,74,93,28,40),
(4,1,53,101,75,95,28,41),
(4,1,54,103,77,97,29,42),
(4,1,55,106,78,99,29,42),
(4,1,56,108,79,101,29,43),
(4,1,57,110,81,103,29,43),
(4,1,58,112,82,105,30,44),
(4,1,59,115,84,107,30,44),
(4,1,60,117,85,109,30,45),
(4,3,1,17,28,20,20,21),
(4,3,2,17,29,21,21,22),
(4,3,3,18,30,22,21,22),
(4,3,4,18,32,22,22,23),
(4,3,5,19,33,23,22,23),
(4,3,6,19,34,24,23,24),
(4,3,7,19,35,25,23,24),
(4,3,8,20,36,26,24,25),
(4,3,9,20,38,27,24,26),
(4,3,10,21,39,27,25,26),
(4,3,11,21,40,28,25,27),
(4,3,12,22,42,29,26,28),
(4,3,13,22,43,30,27,28),
(4,3,14,23,44,31,27,29),
(4,3,15,23,46,32,28,29),
(4,3,16,24,47,33,28,30),
(4,3,17,24,48,34,29,31),
(4,3,18,25,50,35,30,32),
(4,3,19,25,51,36,30,32),
(4,3,20,26,53,37,31,33),
(4,3,21,26,54,38,32,34),
(4,3,22,27,56,39,32,34),
(4,3,23,27,57,40,33,35),
(4,3,24,28,59,41,34,36),
(4,3,25,28,60,42,34,37),
(4,3,26,29,62,43,35,37),
(4,3,27,29,64,44,36,38),
(4,3,28,30,65,45,36,39),
(4,3,29,30,67,46,37,40),
(4,3,30,31,69,47,38,40),
(4,3,31,31,70,49,39,41),
(4,3,32,32,72,50,39,42),
(4,3,33,33,74,51,40,43),
(4,3,34,33,75,52,41,44),
(4,3,35,34,77,53,42,45),
(4,3,36,35,79,55,43,46),
(4,3,37,35,81,56,43,46),
(4,3,38,36,83,57,44,47),
(4,3,39,36,85,58,45,48),
(4,3,40,37,86,60,46,49),
(4,3,41,38,88,61,47,50),
(4,3,42,38,90,62,47,51),
(4,3,43,39,92,63,48,52),
(4,3,44,40,94,65,49,53),
(4,3,45,40,96,66,50,54),
(4,3,46,41,98,68,51,55),
(4,3,47,42,100,69,52,56),
(4,3,48,43,103,70,53,57),
(4,3,49,43,105,72,54,58),
(4,3,50,44,107,73,55,59),
(4,3,51,45,109,75,56,60),
(4,3,52,46,111,76,57,61),
(4,3,53,46,113,78,58,62),
(4,3,54,47,116,79,59,63),
(4,3,55,48,118,81,60,64),
(4,3,56,49,120,82,61,65),
(4,3,57,50,123,84,62,67),
(4,3,58,50,125,86,63,68),
(4,3,59,51,128,87,64,69),
(4,3,60,52,130,89,65,70),
(4,4,1,18,28,20,20,20),
(4,4,2,19,29,21,20,20),
(4,4,3,19,30,21,20,21),
(4,4,4,20,32,22,21,21),
(4,4,5,21,33,23,21,21),
(4,4,6,21,34,23,21,22),
(4,4,7,22,36,24,21,22),
(4,4,8,23,37,24,21,22),
(4,4,9,24,38,25,21,23),
(4,4,10,24,40,26,22,23),
(4,4,11,25,41,27,22,24),
(4,4,12,26,42,27,22,24),
(4,4,13,27,44,28,22,24),
(4,4,14,27,45,29,22,25),
(4,4,15,28,47,29,23,25),
(4,4,16,29,48,30,23,26),
(4,4,17,30,49,31,23,26),
(4,4,18,31,51,32,23,26),
(4,4,19,32,53,32,23,27),
(4,4,20,32,54,33,24,27),
(4,4,21,33,56,34,24,28),
(4,4,22,34,57,35,24,28),
(4,4,23,35,59,36,24,29),
(4,4,24,36,60,36,25,29),
(4,4,25,37,62,37,25,30),
(4,4,26,38,64,38,25,30),
(4,4,27,39,65,39,25,30),
(4,4,28,40,67,40,25,31),
(4,4,29,40,69,41,26,31),
(4,4,30,41,71,41,26,32),
(4,4,31,42,72,42,26,32),
(4,4,32,43,74,43,26,33),
(4,4,33,44,76,44,27,33),
(4,4,34,45,78,45,27,34),
(4,4,35,46,80,46,27,34),
(4,4,36,48,82,47,28,35),
(4,4,37,49,83,48,28,36),
(4,4,38,50,85,49,28,36),
(4,4,39,51,87,50,28,37),
(4,4,40,52,89,51,29,37),
(4,4,41,53,91,52,29,38),
(4,4,42,54,93,53,29,38),
(4,4,43,55,95,54,29,39),
(4,4,44,56,98,55,30,39),
(4,4,45,58,100,56,30,40),
(4,4,46,59,102,57,30,41),
(4,4,47,60,104,58,31,41),
(4,4,48,61,106,59,31,42),
(4,4,49,62,108,61,31,43),
(4,4,50,64,111,62,32,43),
(4,4,51,65,113,63,32,44),
(4,4,52,66,115,64,32,44),
(4,4,53,67,118,65,33,45),
(4,4,54,69,120,66,33,46),
(4,4,55,70,122,68,33,46),
(4,4,56,71,125,69,34,47),
(4,4,57,73,127,70,34,48),
(4,4,58,74,130,71,34,49),
(4,4,59,76,132,73,35,49),
(4,4,60,77,135,74,35,50),
(4,5,1,17,25,19,22,23),
(4,5,2,17,25,19,23,24),
(4,5,3,17,25,20,24,25),
(4,5,4,18,26,20,25,27),
(4,5,5,18,26,20,27,28),
(4,5,6,18,26,21,28,29),
(4,5,7,18,26,21,29,30),
(4,5,8,18,27,21,30,31),
(4,5,9,18,27,22,31,33),
(4,5,10,19,27,22,33,34),
(4,5,11,19,27,23,34,35),
(4,5,12,19,28,23,35,37),
(4,5,13,19,28,23,36,38),
(4,5,14,19,28,24,38,39),
(4,5,15,20,28,24,39,41),
(4,5,16,20,29,25,40,42),
(4,5,17,20,29,25,42,43),
(4,5,18,20,29,25,43,45),
(4,5,19,20,30,26,44,46),
(4,5,20,21,30,26,46,48),
(4,5,21,21,30,27,47,49),
(4,5,22,21,30,27,49,51),
(4,5,23,21,31,28,50,52),
(4,5,24,22,31,28,52,54),
(4,5,25,22,31,29,53,55),
(4,5,26,22,32,29,55,57),
(4,5,27,22,32,29,56,59),
(4,5,28,22,32,30,58,60),
(4,5,29,23,33,30,59,62),
(4,5,30,23,33,31,61,64),
(4,5,31,23,33,31,63,65),
(4,5,32,23,34,32,64,67),
(4,5,33,24,34,32,66,69),
(4,5,34,24,34,33,68,70),
(4,5,35,24,35,33,69,72),
(4,5,36,25,35,34,71,74),
(4,5,37,25,35,35,73,76),
(4,5,38,25,36,35,75,78),
(4,5,39,25,36,36,76,80),
(4,5,40,26,36,36,78,81),
(4,5,41,26,37,37,80,83),
(4,5,42,26,37,37,82,85),
(4,5,43,26,38,38,84,87),
(4,5,44,27,38,38,86,89),
(4,5,45,27,38,39,88,91),
(4,5,46,27,39,40,90,93),
(4,5,47,28,39,40,92,95),
(4,5,48,28,40,41,94,98),
(4,5,49,28,40,42,96,100),
(4,5,50,29,40,42,98,102),
(4,5,51,29,41,43,100,104),
(4,5,52,29,41,43,102,106),
(4,5,53,30,42,44,104,108),
(4,5,54,30,42,45,106,111),
(4,5,55,30,43,45,109,113),
(4,5,56,31,43,46,111,115),
(4,5,57,31,44,47,113,118),
(4,5,58,31,44,48,115,120),
(4,5,59,32,45,48,118,123),
(4,5,60,32,45,49,120,125),
(4,11,1,18,25,19,22,22),
(4,11,2,19,25,20,23,23),
(4,11,3,19,26,20,24,24),
(4,11,4,20,26,21,25,25),
(4,11,5,20,27,21,26,26),
(4,11,6,21,27,22,27,27),
(4,11,7,21,28,23,28,28),
(4,11,8,22,28,23,28,29),
(4,11,9,22,29,24,29,30),
(4,11,10,23,29,24,30,31),
(4,11,11,23,30,25,31,33),
(4,11,12,24,30,26,32,34),
(4,11,13,24,31,26,33,35),
(4,11,14,25,31,27,34,36),
(4,11,15,26,32,28,36,37),
(4,11,16,26,32,28,37,38),
(4,11,17,27,33,29,38,40),
(4,11,18,27,34,30,39,41),
(4,11,19,28,34,30,40,42),
(4,11,20,29,35,31,41,43),
(4,11,21,29,35,32,42,45),
(4,11,22,30,36,33,43,46),
(4,11,23,31,37,33,44,47),
(4,11,24,31,37,34,46,49),
(4,11,25,32,38,35,47,50),
(4,11,26,33,38,36,48,51),
(4,11,27,33,39,36,49,53),
(4,11,28,34,40,37,50,54),
(4,11,29,35,40,38,52,56),
(4,11,30,35,41,39,53,57),
(4,11,31,36,42,40,54,58),
(4,11,32,37,42,41,56,60),
(4,11,33,38,43,41,57,61),
(4,11,34,38,44,42,58,63),
(4,11,35,39,44,43,60,64),
(4,11,36,40,45,44,61,66),
(4,11,37,41,46,45,62,68),
(4,11,38,42,46,46,64,69),
(4,11,39,42,47,47,65,71),
(4,11,40,43,48,48,67,72),
(4,11,41,44,49,49,68,74),
(4,11,42,45,49,50,70,76),
(4,11,43,46,50,51,71,77),
(4,11,44,47,51,51,73,79),
(4,11,45,47,52,52,74,81),
(4,11,46,48,53,53,76,83),
(4,11,47,49,53,55,77,84),
(4,11,48,50,54,56,79,86),
(4,11,49,51,55,57,81,88),
(4,11,50,52,56,58,82,90),
(4,11,51,53,57,59,84,92),
(4,11,52,54,58,60,86,94),
(4,11,53,55,59,61,87,96),
(4,11,54,56,59,62,89,98),
(4,11,55,57,60,63,91,100),
(4,11,56,58,61,64,93,102),
(4,11,57,59,62,65,94,104),
(4,11,58,60,63,67,96,106),
(4,11,59,61,64,68,98,108),
(4,11,60,62,65,69,100,110),
(5,1,1,22,18,23,18,25),
(5,1,2,23,19,24,18,25),
(5,1,3,24,19,25,18,26),
(5,1,4,25,20,26,18,26),
(5,1,5,27,21,27,18,26),
(5,1,6,28,22,28,19,26),
(5,1,7,29,22,29,19,27),
(5,1,8,30,23,30,19,27),
(5,1,9,31,24,31,19,27),
(5,1,10,32,24,32,19,28),
(5,1,11,34,25,34,19,28),
(5,1,12,35,26,35,19,28),
(5,1,13,36,27,36,19,29),
(5,1,14,38,28,37,20,29),
(5,1,15,39,28,38,20,29),
(5,1,16,40,29,39,20,30),
(5,1,17,41,30,41,20,30),
(5,1,18,43,31,42,20,30),
(5,1,19,44,32,43,20,31),
(5,1,20,46,33,44,20,31),
(5,1,21,47,33,46,21,31),
(5,1,22,48,34,47,21,32),
(5,1,23,50,35,48,21,32),
(5,1,24,51,36,50,21,33),
(5,1,25,53,37,51,21,33),
(5,1,26,54,38,52,21,33),
(5,1,27,56,39,54,21,34),
(5,1,28,57,40,55,22,34),
(5,1,29,59,41,57,22,35),
(5,1,30,61,42,58,22,35),
(5,1,31,62,43,59,22,35),
(5,1,32,64,44,61,22,36),
(5,1,33,65,45,62,22,36),
(5,1,34,67,46,64,23,37),
(5,1,35,69,47,65,23,37),
(5,1,36,71,48,67,23,38),
(5,1,37,72,49,69,23,38),
(5,1,38,74,50,70,23,38),
(5,1,39,76,51,72,24,39),
(5,1,40,78,52,73,24,39),
(5,1,41,79,54,75,24,40),
(5,1,42,81,55,77,24,40),
(5,1,43,83,56,78,24,41),
(5,1,44,85,57,80,24,41),
(5,1,45,87,58,82,25,42),
(5,1,46,89,59,84,25,42),
(5,1,47,91,61,85,25,43),
(5,1,48,93,62,87,25,43),
(5,1,49,95,63,89,26,44),
(5,1,50,97,64,91,26,44),
(5,1,51,99,66,93,26,45),
(5,1,52,101,67,95,26,45),
(5,1,53,103,68,97,26,46),
(5,1,54,105,70,99,27,47),
(5,1,55,108,71,101,27,47),
(5,1,56,110,72,103,27,48),
(5,1,57,112,74,105,27,48),
(5,1,58,114,75,107,28,49),
(5,1,59,117,77,109,28,49),
(5,1,60,119,78,111,28,50),
(5,4,1,20,21,22,18,25),
(5,4,2,21,22,23,18,25),
(5,4,3,21,23,23,18,26),
(5,4,4,22,25,24,19,26),
(5,4,5,23,26,25,19,26),
(5,4,6,23,27,25,19,27),
(5,4,7,24,29,26,19,27),
(5,4,8,25,30,26,19,27),
(5,4,9,26,31,27,19,28),
(5,4,10,26,33,28,20,28),
(5,4,11,27,34,29,20,29),
(5,4,12,28,35,29,20,29),
(5,4,13,29,37,30,20,29),
(5,4,14,29,38,31,20,30),
(5,4,15,30,40,31,21,30),
(5,4,16,31,41,32,21,31),
(5,4,17,32,42,33,21,31),
(5,4,18,33,44,34,21,31),
(5,4,19,34,46,34,21,32),
(5,4,20,34,47,35,22,32),
(5,4,21,35,49,36,22,33),
(5,4,22,36,50,37,22,33),
(5,4,23,37,52,38,22,34),
(5,4,24,38,53,38,23,34),
(5,4,25,39,55,39,23,35),
(5,4,26,40,57,40,23,35),
(5,4,27,41,58,41,23,35),
(5,4,28,42,60,42,23,36),
(5,4,29,42,62,43,24,36),
(5,4,30,43,64,43,24,37),
(5,4,31,44,65,44,24,37),
(5,4,32,45,67,45,24,38),
(5,4,33,46,69,46,25,38),
(5,4,34,47,71,47,25,39),
(5,4,35,48,73,48,25,39),
(5,4,36,50,75,49,26,40),
(5,4,37,51,76,50,26,41),
(5,4,38,52,78,51,26,41),
(5,4,39,53,80,52,26,42),
(5,4,40,54,82,53,27,42),
(5,4,41,55,84,54,27,43),
(5,4,42,56,86,55,27,43),
(5,4,43,57,88,56,27,44),
(5,4,44,58,91,57,28,44),
(5,4,45,60,93,58,28,45),
(5,4,46,61,95,59,28,46),
(5,4,47,62,97,60,29,46),
(5,4,48,63,99,61,29,47),
(5,4,49,64,101,63,29,48),
(5,4,50,66,104,64,30,48),
(5,4,51,67,106,65,30,49),
(5,4,52,68,108,66,30,49),
(5,4,53,69,111,67,31,50),
(5,4,54,71,113,68,31,51),
(5,4,55,72,115,70,31,51),
(5,4,56,73,118,71,32,52),
(5,4,57,75,120,72,32,53),
(5,4,58,76,123,73,32,54),
(5,4,59,78,125,75,33,54),
(5,4,60,79,128,76,33,55),
(5,5,1,19,18,21,20,28),
(5,5,2,19,18,21,21,29),
(5,5,3,19,18,22,22,30),
(5,5,4,20,19,22,23,32),
(5,5,5,20,19,22,25,33),
(5,5,6,20,19,23,26,34),
(5,5,7,20,19,23,27,35),
(5,5,8,20,20,23,28,36),
(5,5,9,20,20,24,29,38),
(5,5,10,21,20,24,31,39),
(5,5,11,21,20,25,32,40),
(5,5,12,21,21,25,33,42),
(5,5,13,21,21,25,34,43),
(5,5,14,21,21,26,36,44),
(5,5,15,22,21,26,37,46),
(5,5,16,22,22,27,38,47),
(5,5,17,22,22,27,40,48),
(5,5,18,22,22,27,41,50),
(5,5,19,22,23,28,42,51),
(5,5,20,23,23,28,44,53),
(5,5,21,23,23,29,45,54),
(5,5,22,23,23,29,47,56),
(5,5,23,23,24,30,48,57),
(5,5,24,24,24,30,50,59),
(5,5,25,24,24,31,51,60),
(5,5,26,24,25,31,53,62),
(5,5,27,24,25,31,54,64),
(5,5,28,24,25,32,56,65),
(5,5,29,25,26,32,57,67),
(5,5,30,25,26,33,59,69),
(5,5,31,25,26,33,61,70),
(5,5,32,25,27,34,62,72),
(5,5,33,26,27,34,64,74),
(5,5,34,26,27,35,66,75),
(5,5,35,26,28,35,67,77),
(5,5,36,27,28,36,69,79),
(5,5,37,27,28,37,71,81),
(5,5,38,27,29,37,73,83),
(5,5,39,27,29,38,74,85),
(5,5,40,28,29,38,76,86),
(5,5,41,28,30,39,78,88),
(5,5,42,28,30,39,80,90),
(5,5,43,28,31,40,82,92),
(5,5,44,29,31,40,84,94),
(5,5,45,29,31,41,86,96),
(5,5,46,29,32,42,88,98),
(5,5,47,30,32,42,90,100),
(5,5,48,30,33,43,92,103),
(5,5,49,30,33,44,94,105),
(5,5,50,31,33,44,96,107),
(5,5,51,31,34,45,98,109),
(5,5,52,31,34,45,100,111),
(5,5,53,32,35,46,102,113),
(5,5,54,32,35,47,104,116),
(5,5,55,32,36,47,107,118),
(5,5,56,33,36,48,109,120),
(5,5,57,33,37,49,111,123),
(5,5,58,33,37,50,113,125),
(5,5,59,34,38,50,116,128),
(5,5,60,34,38,51,118,130),
(5,8,1,19,18,21,27,21),
(5,8,2,19,18,21,22,28),
(5,8,3,19,18,22,23,29),
(5,8,4,19,19,22,25,30),
(5,8,5,19,19,22,26,32),
(5,8,6,20,19,22,27,33),
(5,8,7,20,19,23,28,34),
(5,8,8,20,19,23,29,35),
(5,8,9,20,19,23,31,36),
(5,8,10,20,20,24,32,38),
(5,8,11,20,20,24,33,39),
(5,8,12,20,20,24,35,40),
(5,8,13,20,20,25,36,41),
(5,8,14,21,20,25,37,43),
(5,8,15,21,21,25,39,44),
(5,8,16,21,21,26,40,45),
(5,8,17,21,21,26,41,47),
(5,8,18,21,21,26,43,48),
(5,8,19,21,21,27,44,49),
(5,8,20,21,22,27,46,51),
(5,8,21,22,22,27,47,52),
(5,8,22,22,22,28,49,54),
(5,8,23,22,22,28,50,55),
(5,8,24,22,23,29,52,57),
(5,8,25,22,23,29,53,58),
(5,8,26,22,23,29,55,60),
(5,8,27,22,23,30,57,61),
(5,8,28,23,23,30,58,63),
(5,8,29,23,24,31,60,64),
(5,8,30,23,24,31,62,66),
(5,8,31,23,24,31,63,68),
(5,8,32,23,24,32,65,69),
(5,8,33,23,25,32,67,71),
(5,8,34,24,25,33,68,73),
(5,8,35,24,25,33,70,74),
(5,8,36,24,26,34,72,76),
(5,8,37,24,26,34,74,78),
(5,8,38,24,26,34,76,80),
(5,8,39,25,26,35,78,81),
(5,8,40,25,27,35,79,83),
(5,8,41,25,27,36,81,85),
(5,8,42,25,27,36,83,87),
(5,8,43,25,27,37,85,89),
(5,8,44,25,28,37,87,91),
(5,8,45,26,28,38,89,93),
(5,8,46,26,28,38,91,95),
(5,8,47,26,29,39,93,97),
(5,8,48,26,29,39,96,99),
(5,8,49,27,29,40,98,101),
(5,8,50,27,30,40,100,103),
(5,8,51,27,30,41,102,105),
(5,8,52,27,30,41,104,107),
(5,8,53,27,31,42,106,109),
(5,8,54,28,31,43,109,111),
(5,8,55,28,31,43,111,114),
(5,8,56,28,32,44,113,116),
(5,8,57,28,32,44,116,118),
(5,8,58,29,32,45,118,120),
(5,8,59,29,33,45,121,123),
(5,8,60,29,33,46,123,125),
(5,9,1,19,18,22,20,27),
(5,9,2,19,18,23,21,28),
(5,9,3,20,19,23,22,29),
(5,9,4,20,19,24,23,30),
(5,9,5,20,19,24,24,31),
(5,9,6,20,20,25,25,32),
(5,9,7,21,20,25,26,34),
(5,9,8,21,20,26,27,35),
(5,9,9,21,21,26,28,36),
(5,9,10,22,21,27,29,37),
(5,9,11,22,22,27,31,38),
(5,9,12,22,22,28,32,39),
(5,9,13,23,22,28,33,41),
(5,9,14,23,23,29,34,42),
(5,9,15,23,23,30,35,43),
(5,9,16,24,24,30,36,44),
(5,9,17,24,24,31,38,46),
(5,9,18,24,24,31,39,47),
(5,9,19,25,25,32,40,48),
(5,9,20,25,25,33,41,50),
(5,9,21,25,26,33,43,51),
(5,9,22,26,26,34,44,52),
(5,9,23,26,27,35,45,54),
(5,9,24,27,27,35,47,55),
(5,9,25,27,28,36,48,57),
(5,9,26,27,28,37,49,58),
(5,9,27,28,28,37,51,59),
(5,9,28,28,29,38,52,61),
(5,9,29,29,29,39,54,62),
(5,9,30,29,30,39,55,64),
(5,9,31,29,30,40,56,66),
(5,9,32,30,31,41,58,67),
(5,9,33,30,31,42,59,69),
(5,9,34,31,32,42,61,70),
(5,9,35,31,32,43,62,72),
(5,9,36,32,33,43,64,75),
(5,9,37,32,34,45,66,75),
(5,9,38,32,34,46,67,77),
(5,9,39,33,35,46,69,79),
(5,9,40,33,35,47,70,80),
(5,9,41,34,36,48,72,82),
(5,9,42,34,36,48,74,85),
(5,9,43,35,37,50,75,86),
(5,9,44,35,37,51,77,87),
(5,9,45,36,38,51,79,89),
(5,9,46,36,39,52,81,90),
(5,9,47,37,39,53,82,93),
(5,9,48,37,40,54,84,95),
(5,9,49,38,41,55,86,96),
(5,9,50,38,41,56,88,99),
(5,9,51,39,42,57,90,101),
(5,9,52,39,42,58,92,103),
(5,9,53,40,43,59,94,105),
(5,9,54,41,44,60,96,107),
(5,9,55,41,44,60,98,110),
(5,9,56,42,45,62,100,111),
(5,9,57,42,46,63,102,113),
(5,9,58,43,47,63,104,116),
(5,9,59,43,47,65,106,118),
(5,9,60,44,48,66,108,120),
(6,1,1,28,15,24,15,22),
(6,1,2,29,16,25,15,22),
(6,1,3,30,16,26,15,23),
(6,1,4,31,17,27,15,23),
(6,1,5,33,18,28,15,23),
(6,1,6,34,19,29,16,23),
(6,1,7,35,19,30,16,24),
(6,1,8,36,20,31,16,24),
(6,1,9,37,21,32,16,24),
(6,1,10,38,21,33,16,25),
(6,1,11,40,22,35,16,25),
(6,1,12,41,23,36,16,25),
(6,1,13,42,24,37,16,26),
(6,1,14,44,25,38,17,26),
(6,1,15,45,25,39,17,26),
(6,1,16,46,26,40,17,27),
(6,1,17,47,27,42,17,27),
(6,1,18,49,28,43,17,27),
(6,1,19,50,29,44,17,28),
(6,1,20,52,30,45,17,28),
(6,1,21,53,30,47,18,28),
(6,1,22,54,31,48,18,29),
(6,1,23,56,32,49,18,29),
(6,1,24,57,33,51,18,30),
(6,1,25,59,34,52,18,30),
(6,1,26,60,35,53,18,30),
(6,1,27,62,36,55,18,31),
(6,1,28,63,37,56,19,31),
(6,1,29,65,38,58,19,32),
(6,1,30,67,39,59,19,32),
(6,1,31,68,40,60,19,32),
(6,1,32,70,41,62,19,33),
(6,1,33,71,42,63,19,33),
(6,1,34,73,43,65,20,34),
(6,1,35,75,44,66,20,34),
(6,1,36,77,45,68,20,35),
(6,1,37,78,46,70,20,35),
(6,1,38,80,47,71,20,35),
(6,1,39,82,48,73,21,36),
(6,1,40,84,49,74,21,36),
(6,1,41,85,51,76,21,37),
(6,1,42,87,52,78,21,37),
(6,1,43,89,53,79,21,38),
(6,1,44,91,54,81,21,38),
(6,1,45,93,55,83,22,39),
(6,1,46,95,56,85,22,39),
(6,1,47,97,58,86,22,40),
(6,1,48,99,59,88,22,40),
(6,1,49,101,60,90,23,41),
(6,1,50,103,61,92,23,41),
(6,1,51,105,63,94,23,42),
(6,1,52,107,64,96,23,42),
(6,1,53,109,65,98,23,43),
(6,1,54,111,67,100,24,44),
(6,1,55,114,68,102,24,44),
(6,1,56,116,69,104,24,45),
(6,1,57,118,71,106,24,45),
(6,1,58,120,72,108,25,46),
(6,1,59,123,74,110,25,46),
(6,1,60,125,75,112,25,47),
(6,3,1,25,18,23,15,23),
(6,3,2,25,19,24,16,24),
(6,3,3,26,20,25,16,24),
(6,3,4,26,22,25,17,25),
(6,3,5,27,23,26,17,25),
(6,3,6,27,24,27,18,26),
(6,3,7,27,25,28,18,26),
(6,3,8,28,26,29,19,27),
(6,3,9,28,28,30,19,28),
(6,3,10,29,29,30,20,28),
(6,3,11,29,30,31,20,29),
(6,3,12,30,32,32,21,30),
(6,3,13,30,33,33,22,30),
(6,3,14,31,34,34,22,31),
(6,3,15,31,36,35,23,31),
(6,3,16,32,37,36,23,32),
(6,3,17,32,38,37,24,33),
(6,3,18,33,40,38,25,34),
(6,3,19,33,41,39,25,34),
(6,3,20,34,43,40,26,35),
(6,3,21,34,44,41,27,36),
(6,3,22,35,46,42,27,36),
(6,3,23,35,47,43,28,37),
(6,3,24,36,49,44,29,38),
(6,3,25,36,50,45,29,39),
(6,3,26,37,52,46,30,39),
(6,3,27,37,54,47,31,40),
(6,3,28,38,55,48,31,41),
(6,3,29,38,57,49,32,42),
(6,3,30,39,59,50,33,42),
(6,3,31,39,60,52,34,43),
(6,3,32,40,62,53,34,44),
(6,3,33,41,64,54,35,45),
(6,3,34,41,65,55,36,46),
(6,3,35,42,67,56,37,47),
(6,3,36,43,69,58,38,48),
(6,3,37,43,71,59,38,48),
(6,3,38,44,73,60,39,49),
(6,3,39,44,75,61,40,50),
(6,3,40,45,76,63,41,51),
(6,3,41,46,78,64,42,52),
(6,3,42,46,80,65,42,53),
(6,3,43,47,82,66,43,54),
(6,3,44,48,84,68,44,55),
(6,3,45,48,86,69,45,56),
(6,3,46,49,88,71,46,57),
(6,3,47,50,90,72,47,58),
(6,3,48,51,93,73,48,59),
(6,3,49,51,95,75,49,60),
(6,3,50,52,97,76,50,61),
(6,3,51,53,99,78,51,62),
(6,3,52,54,101,79,52,63),
(6,3,53,54,103,81,53,64),
(6,3,54,55,106,82,54,65),
(6,3,55,56,108,84,55,66),
(6,3,56,57,110,85,56,67),
(6,3,57,58,113,87,57,69),
(6,3,58,58,115,89,58,70),
(6,3,59,59,118,90,59,71),
(6,3,60,60,120,92,60,72),
(6,7,1,26,15,23,16,24),
(6,7,2,27,15,24,17,25),
(6,7,3,27,16,25,18,26),
(6,7,4,28,16,26,18,27),
(6,7,5,29,17,26,19,28),
(6,7,6,30,17,27,20,29),
(6,7,7,31,17,28,21,30),
(6,7,8,31,18,29,22,30),
(6,7,9,32,18,30,23,31),
(6,7,10,33,19,31,23,32),
(6,7,11,34,19,32,24,33),
(6,7,12,35,20,33,25,34),
(6,7,13,35,20,34,26,35),
(6,7,14,36,21,35,27,36),
(6,7,15,37,21,36,28,38),
(6,7,16,38,22,37,29,39),
(6,7,17,39,22,38,30,40),
(6,7,18,40,23,39,31,41),
(6,7,19,41,23,40,32,42),
(6,7,20,42,24,41,33,43),
(6,7,21,43,24,42,34,44),
(6,7,22,43,25,43,35,45),
(6,7,23,44,25,44,36,46),
(6,7,24,45,26,45,37,48),
(6,7,25,46,26,47,38,49),
(6,7,26,47,27,48,39,50),
(6,7,27,48,27,49,40,51),
(6,7,28,49,28,50,41,52),
(6,7,29,50,28,51,42,54),
(6,7,30,51,29,52,43,55),
(6,7,31,53,29,54,45,56),
(6,7,32,54,30,55,46,58),
(6,7,33,55,31,56,47,59),
(6,7,34,56,31,57,48,60),
(6,7,35,57,32,59,49,62),
(6,7,36,58,33,60,51,63),
(6,7,37,59,33,61,52,64),
(6,7,38,60,34,63,53,66),
(6,7,39,61,34,64,54,67),
(6,7,40,63,35,65,56,69),
(6,7,41,64,36,67,57,70),
(6,7,42,65,36,68,58,72),
(6,7,43,66,37,70,59,73),
(6,7,44,68,38,71,61,75),
(6,7,45,69,38,73,62,76),
(6,7,46,70,39,74,64,78),
(6,7,47,71,40,76,65,79),
(6,7,48,73,41,77,66,81),
(6,7,49,74,41,79,68,83),
(6,7,50,75,42,80,69,84),
(6,7,51,77,43,82,71,86),
(6,7,52,78,44,83,72,88),
(6,7,53,80,44,85,74,89),
(6,7,54,81,45,87,75,91),
(6,7,55,82,46,88,77,93),
(6,7,56,84,47,90,78,95),
(6,7,57,85,48,92,80,96),
(6,7,58,87,48,93,82,98),
(6,7,59,88,49,95,83,100),
(6,7,60,90,50,97,85,102),
(6,11,1,26,15,22,17,24),
(6,11,2,27,15,23,18,25),
(6,11,3,27,16,23,19,26),
(6,11,4,28,16,24,20,27),
(6,11,5,28,17,24,21,28),
(6,11,6,29,17,25,22,29),
(6,11,7,29,18,26,23,30),
(6,11,8,30,18,26,23,31),
(6,11,9,30,19,27,24,32),
(6,11,10,31,19,27,25,33),
(6,11,11,31,20,28,26,35),
(6,11,12,32,20,29,27,36),
(6,11,13,32,21,29,28,37),
(6,11,14,33,21,30,29,38),
(6,11,15,34,22,31,31,39),
(6,11,16,34,22,31,32,40),
(6,11,17,35,23,32,33,42),
(6,11,18,35,24,33,34,43),
(6,11,19,36,24,33,35,44),
(6,11,20,37,25,34,36,45),
(6,11,21,37,25,35,37,47),
(6,11,22,38,26,36,38,48),
(6,11,23,39,27,36,39,49),
(6,11,24,39,27,37,41,51),
(6,11,25,40,28,38,42,52),
(6,11,26,41,28,39,43,53),
(6,11,27,41,29,39,44,55),
(6,11,28,42,30,40,45,56),
(6,11,29,43,30,41,47,58),
(6,11,30,43,31,42,48,59),
(6,11,31,44,32,43,49,60),
(6,11,32,45,32,44,51,62),
(6,11,33,46,33,44,52,63),
(6,11,34,46,34,45,53,65),
(6,11,35,47,34,46,55,66),
(6,11,36,48,35,47,56,68),
(6,11,37,49,36,48,57,70),
(6,11,38,50,36,49,59,71),
(6,11,39,50,37,50,60,73),
(6,11,40,51,38,51,62,74),
(6,11,41,52,39,52,63,76),
(6,11,42,53,39,53,65,78),
(6,11,43,54,40,54,66,79),
(6,11,44,55,41,54,68,81),
(6,11,45,55,42,55,69,83),
(6,11,46,56,43,56,71,85),
(6,11,47,57,43,58,72,86),
(6,11,48,58,44,59,74,88),
(6,11,49,59,45,60,76,90),
(6,11,50,60,46,61,77,92),
(6,11,51,61,47,62,79,94),
(6,11,52,62,48,63,81,96),
(6,11,53,63,49,64,82,98),
(6,11,54,64,49,65,84,100),
(6,11,55,65,50,66,86,102),
(6,11,56,66,51,67,88,104),
(6,11,57,67,52,68,89,106),
(6,11,58,68,53,70,91,108),
(6,11,59,69,54,71,93,110),
(6,11,60,70,55,72,95,112),
(7,1,1,18,23,21,23,20),
(7,1,2,19,24,22,24,20),
(7,1,3,20,24,23,24,21),
(7,1,4,21,25,24,24,21),
(7,1,5,23,26,25,24,21),
(7,1,6,24,27,26,25,21),
(7,1,7,25,27,27,25,22),
(7,1,8,26,28,28,25,22),
(7,1,9,27,29,29,25,22),
(7,1,10,28,29,30,25,23),
(7,1,11,30,30,32,25,23),
(7,1,12,31,31,33,25,23),
(7,1,13,32,32,34,25,24),
(7,1,14,34,33,35,26,24),
(7,1,15,35,33,36,26,24),
(7,1,16,36,34,37,26,25),
(7,1,17,37,35,39,26,25),
(7,1,18,39,36,40,26,25),
(7,1,19,40,37,41,26,26),
(7,1,20,42,38,42,26,26),
(7,1,21,43,38,44,27,26),
(7,1,22,44,39,45,27,27),
(7,1,23,46,40,46,27,27),
(7,1,24,47,41,48,27,28),
(7,1,25,49,42,49,27,28),
(7,1,26,50,43,50,27,28),
(7,1,27,52,44,52,27,29),
(7,1,28,53,45,53,28,29),
(7,1,29,55,46,55,28,30),
(7,1,30,57,47,56,28,30),
(7,1,31,58,48,57,28,30),
(7,1,32,60,49,59,28,31),
(7,1,33,61,50,60,28,31),
(7,1,34,63,51,62,29,32),
(7,1,35,65,52,63,29,32),
(7,1,36,67,53,65,29,33),
(7,1,37,68,54,67,29,33),
(7,1,38,70,55,68,29,33),
(7,1,39,72,56,70,30,34),
(7,1,40,74,57,71,30,34),
(7,1,41,75,59,73,30,35),
(7,1,42,77,60,75,30,35),
(7,1,43,79,61,76,30,36),
(7,1,44,81,62,78,30,36),
(7,1,45,83,63,80,32,37),
(7,1,46,85,64,82,32,37),
(7,1,47,87,66,83,32,38),
(7,1,48,89,67,85,32,38),
(7,1,49,91,68,87,33,39),
(7,1,50,93,69,89,33,39),
(7,1,51,95,71,91,33,40),
(7,1,52,97,72,93,33,40),
(7,1,53,99,73,95,33,41),
(7,1,54,101,75,97,34,42),
(7,1,55,104,76,99,34,42),
(7,1,56,106,77,101,34,43),
(7,1,57,108,79,103,34,43),
(7,1,58,110,80,105,35,44),
(7,1,59,113,82,107,35,44),
(7,1,60,115,83,109,35,45),
(7,4,1,16,26,20,23,20),
(7,4,2,17,27,21,24,20),
(7,4,3,17,28,21,24,21),
(7,4,4,18,30,22,25,21),
(7,4,5,19,31,23,25,21),
(7,4,6,19,32,23,25,22),
(7,4,7,20,34,24,25,22),
(7,4,8,21,35,24,25,22),
(7,4,9,22,36,25,25,23),
(7,4,10,22,38,26,26,23),
(7,4,11,23,39,27,26,24),
(7,4,12,24,40,27,26,24),
(7,4,13,25,42,28,26,24),
(7,4,14,25,43,29,26,25),
(7,4,15,26,45,29,27,25),
(7,4,16,27,46,30,27,26),
(7,4,17,28,47,31,27,26),
(7,4,18,29,49,32,27,26),
(7,4,19,30,51,32,27,27),
(7,4,20,30,52,33,28,27),
(7,4,21,31,54,34,28,28),
(7,4,22,32,55,35,28,28),
(7,4,23,33,57,36,28,29),
(7,4,24,34,58,36,29,29),
(7,4,25,35,60,37,29,30),
(7,4,26,36,62,38,29,30),
(7,4,27,37,63,39,29,30),
(7,4,28,38,65,40,29,31),
(7,4,29,38,67,41,30,31),
(7,4,30,39,69,41,30,32),
(7,4,31,40,70,42,30,32),
(7,4,32,41,72,43,30,33),
(7,4,33,42,74,44,32,33),
(7,4,34,43,76,45,32,34),
(7,4,35,44,78,46,32,34),
(7,4,36,46,80,47,33,35),
(7,4,37,47,81,48,33,36),
(7,4,38,48,83,49,33,36),
(7,4,39,49,85,50,33,37),
(7,4,40,50,87,51,34,37),
(7,4,41,51,89,52,34,38),
(7,4,42,52,91,53,34,38),
(7,4,43,53,93,54,34,39),
(7,4,44,54,96,55,35,39),
(7,4,45,56,98,56,35,40),
(7,4,46,57,100,57,35,41),
(7,4,47,58,102,58,36,41),
(7,4,48,59,104,59,36,42),
(7,4,49,60,106,61,36,43),
(7,4,50,62,109,62,37,43),
(7,4,51,63,111,63,37,44),
(7,4,52,64,113,64,37,44),
(7,4,53,65,116,65,38,45),
(7,4,54,67,118,66,38,46),
(7,4,55,68,120,68,38,46),
(7,4,56,69,123,69,39,47),
(7,4,57,71,125,70,39,48),
(7,4,58,72,128,71,39,49),
(7,4,59,74,130,73,40,49),
(7,4,60,75,133,74,40,50),
(7,8,1,15,23,19,26,22),
(7,8,2,15,23,19,28,23),
(7,8,3,15,23,20,29,24),
(7,8,4,15,24,20,32,25),
(7,8,5,15,24,20,33,27),
(7,8,6,16,24,20,34,28),
(7,8,7,16,24,21,35,29),
(7,8,8,16,24,21,36,30),
(7,8,9,16,24,21,38,31),
(7,8,10,16,25,22,39,33),
(7,8,11,16,25,22,40,34),
(7,8,12,16,25,22,42,35),
(7,8,13,16,25,23,43,36),
(7,8,14,17,25,23,44,38),
(7,8,15,17,26,23,46,39),
(7,8,16,17,26,24,47,40),
(7,8,17,17,26,24,48,42),
(7,8,18,17,26,24,50,43),
(7,8,19,17,26,25,51,44),
(7,8,20,17,27,25,54,46),
(7,8,21,18,27,25,55,47),
(7,8,22,18,27,26,57,49),
(7,8,23,18,27,26,58,50),
(7,8,24,18,28,27,60,52),
(7,8,25,18,28,27,61,53),
(7,8,26,18,28,27,63,55),
(7,8,27,18,28,28,65,56),
(7,8,28,19,28,28,66,58),
(7,8,29,19,29,29,68,59),
(7,8,30,19,29,29,70,61),
(7,8,31,19,29,29,71,63),
(7,8,32,19,29,30,74,64),
(7,8,33,19,30,30,76,66),
(7,8,34,20,30,31,77,68),
(7,8,35,20,30,31,79,69),
(7,8,36,20,31,32,81,71),
(7,8,37,20,31,32,83,73),
(7,8,38,20,31,32,85,75),
(7,8,39,21,31,33,87,76),
(7,8,40,21,32,33,88,78),
(7,8,41,21,32,34,90,80),
(7,8,42,21,32,34,92,82),
(7,8,43,21,32,35,95,84),
(7,8,44,21,33,35,97,86),
(7,8,45,22,33,36,99,88),
(7,8,46,22,33,36,101,90),
(7,8,47,22,34,37,103,92),
(7,8,48,22,34,37,106,94),
(7,8,49,23,34,38,108,96),
(7,8,50,23,35,38,110,98),
(7,8,51,23,35,39,112,100),
(7,8,52,23,35,39,114,102),
(7,8,53,23,36,40,117,104),
(7,8,54,24,36,41,120,106),
(7,8,55,24,36,41,122,109),
(7,8,56,24,37,42,124,111),
(7,8,57,24,37,42,127,113),
(7,8,58,25,37,43,129,115),
(7,8,59,25,38,43,132,118),
(7,8,60,25,38,44,133,120),
(7,9,1,15,23,20,25,22),
(7,9,2,15,23,21,27,23),
(7,9,3,16,24,21,28,24),
(7,9,4,16,24,22,29,25),
(7,9,5,16,24,22,30,26),
(7,9,6,16,25,23,32,27),
(7,9,7,17,25,23,33,29),
(7,9,8,17,25,24,34,30),
(7,9,9,17,26,24,35,31),
(7,9,10,18,26,25,36,32),
(7,9,11,18,27,25,38,33),
(7,9,12,18,27,26,39,34),
(7,9,13,19,27,26,40,36),
(7,9,14,19,28,27,41,37),
(7,9,15,19,28,28,42,38),
(7,9,16,20,29,28,43,39),
(7,9,17,20,29,29,45,41),
(7,9,18,20,29,29,46,42),
(7,9,19,21,30,30,47,43),
(7,9,20,21,30,31,48,45),
(7,9,21,21,31,31,50,46),
(7,9,22,22,31,32,51,47),
(7,9,23,22,32,33,53,49),
(7,9,24,23,32,33,55,50),
(7,9,25,23,33,34,56,52),
(7,9,26,23,33,35,57,53),
(7,9,27,24,33,35,59,54),
(7,9,28,24,34,36,60,56),
(7,9,29,25,34,37,62,57),
(7,9,30,25,35,37,63,59),
(7,9,31,25,35,37,64,62),
(7,9,32,26,36,39,66,62),
(7,9,33,26,36,40,67,64),
(7,9,34,27,37,40,69,65),
(7,9,35,27,37,41,70,67),
(7,9,36,28,38,42,72,69),
(7,9,37,28,39,43,75,69),
(7,9,38,28,39,44,76,72),
(7,9,39,29,40,44,78,74),
(7,9,40,29,40,45,79,75),
(7,9,41,30,41,46,81,77),
(7,9,42,30,41,47,83,79),
(7,9,43,31,42,48,84,81),
(7,9,44,31,42,49,86,82),
(7,9,45,32,43,49,88,84),
(7,9,46,32,44,50,90,86),
(7,9,47,33,44,51,91,88),
(7,9,48,33,45,52,93,90),
(7,9,49,34,46,53,96,92),
(7,9,50,34,46,54,98,93),
(7,9,51,35,47,55,100,96),
(7,9,52,35,47,56,102,98),
(7,9,53,36,48,57,104,100),
(7,9,54,37,49,58,106,102),
(7,9,55,37,49,58,108,104),
(7,9,56,38,50,60,110,106),
(7,9,57,38,51,61,112,108),
(7,9,58,39,52,62,114,111),
(7,9,59,39,52,63,117,111),
(7,9,60,40,53,64,119,115),
(8,1,1,24,22,23,16,21),
(8,1,2,25,23,24,16,21),
(8,1,3,26,23,25,16,22),
(8,1,4,27,24,26,16,22),
(8,1,5,29,25,27,16,22),
(8,1,6,30,26,28,17,22),
(8,1,7,31,26,29,17,23),
(8,1,8,32,27,30,17,23),
(8,1,9,33,28,31,17,23),
(8,1,10,34,28,32,17,24),
(8,1,11,36,29,34,17,24),
(8,1,12,37,30,35,17,24),
(8,1,13,38,31,36,17,25),
(8,1,14,40,32,37,18,25),
(8,1,15,41,32,38,18,25),
(8,1,16,42,33,39,18,26),
(8,1,17,43,34,41,18,26),
(8,1,18,45,35,42,18,26),
(8,1,19,46,36,43,18,27),
(8,1,20,48,37,44,18,27),
(8,1,21,49,37,46,19,27),
(8,1,22,50,38,47,19,28),
(8,1,23,52,39,48,19,28),
(8,1,24,53,40,50,19,29),
(8,1,25,55,41,51,19,29),
(8,1,26,56,42,52,19,29),
(8,1,27,58,43,54,19,30),
(8,1,28,59,44,55,20,30),
(8,1,29,61,45,57,20,31),
(8,1,30,63,46,58,20,31),
(8,1,31,64,47,59,20,31),
(8,1,32,66,48,61,20,32),
(8,1,33,67,49,62,20,32),
(8,1,34,69,50,64,21,33),
(8,1,35,71,51,65,21,33),
(8,1,36,73,52,67,21,34),
(8,1,37,74,53,69,21,34),
(8,1,38,76,54,70,21,34),
(8,1,39,78,55,72,22,35),
(8,1,40,80,56,73,22,35),
(8,1,41,81,58,75,22,36),
(8,1,42,83,59,77,22,36),
(8,1,43,85,60,78,22,37),
(8,1,44,87,61,80,22,37),
(8,1,45,89,62,82,23,38),
(8,1,46,91,63,84,23,38),
(8,1,47,93,65,85,23,39),
(8,1,48,95,66,87,23,39),
(8,1,49,97,67,89,24,40),
(8,1,50,99,68,91,24,40),
(8,1,51,101,70,93,24,41),
(8,1,52,103,71,95,24,41),
(8,1,53,105,72,97,24,42),
(8,1,54,107,74,99,25,43),
(8,1,55,110,75,101,25,43),
(8,1,56,112,76,103,25,44),
(8,1,57,114,78,105,25,44),
(8,1,58,116,79,107,26,45),
(8,1,59,119,81,109,26,45),
(8,1,60,121,82,111,26,46),
(8,3,1,21,25,22,16,22),
(8,3,2,21,26,23,17,23),
(8,3,3,22,27,24,17,23),
(8,3,4,22,29,24,18,24),
(8,3,5,23,30,25,18,24),
(8,3,6,23,31,26,19,25),
(8,3,7,23,32,27,19,25),
(8,3,8,24,33,28,20,26),
(8,3,9,24,35,29,20,27),
(8,3,10,25,36,29,21,27),
(8,3,11,25,37,30,21,28),
(8,3,12,26,39,31,22,29),
(8,3,13,26,40,32,23,29),
(8,3,14,27,41,33,23,30),
(8,3,15,27,43,34,24,30),
(8,3,16,28,44,35,24,31),
(8,3,17,28,45,36,25,32),
(8,3,18,29,47,37,26,33),
(8,3,19,29,48,38,26,33),
(8,3,20,30,50,39,27,34),
(8,3,21,30,51,40,28,35),
(8,3,22,31,53,41,28,35),
(8,3,23,31,54,42,29,36),
(8,3,24,32,56,43,30,37),
(8,3,25,32,57,44,30,38),
(8,3,26,33,59,45,31,38),
(8,3,27,33,61,46,32,39),
(8,3,28,34,62,47,32,40),
(8,3,29,34,64,48,33,41),
(8,3,30,35,66,49,34,41),
(8,3,31,35,67,51,35,42),
(8,3,32,36,69,52,35,43),
(8,3,33,37,71,53,36,44),
(8,3,34,37,72,54,37,45),
(8,3,35,38,74,55,38,46),
(8,3,36,39,76,57,39,47),
(8,3,37,39,78,58,39,47),
(8,3,38,40,80,59,40,48),
(8,3,39,40,82,60,41,49),
(8,3,40,41,83,62,42,50),
(8,3,41,42,85,63,43,51),
(8,3,42,42,87,64,43,52),
(8,3,43,43,89,65,44,53),
(8,3,44,44,91,67,45,54),
(8,3,45,44,93,68,46,55),
(8,3,46,45,95,70,47,56),
(8,3,47,46,97,71,48,57),
(8,3,48,47,100,72,49,58),
(8,3,49,47,102,74,50,59),
(8,3,50,48,104,75,51,60),
(8,3,51,49,106,77,52,61),
(8,3,52,50,108,78,53,62),
(8,3,53,50,110,80,54,63),
(8,3,54,51,113,81,55,64),
(8,3,55,52,115,83,56,65),
(8,3,56,53,117,84,57,66),
(8,3,57,54,120,86,58,68),
(8,3,58,54,122,88,59,69),
(8,3,59,55,125,89,60,70),
(8,3,60,56,127,91,61,71),
(8,4,1,22,25,22,16,21),
(8,4,2,23,26,23,16,21),
(8,4,3,23,27,23,16,22),
(8,4,4,24,29,24,17,22),
(8,4,5,25,30,25,17,22),
(8,4,6,25,31,25,17,23),
(8,4,7,26,33,26,17,23),
(8,4,8,27,34,26,17,23),
(8,4,9,28,35,27,17,24),
(8,4,10,28,37,28,18,24),
(8,4,11,29,38,29,18,25),
(8,4,12,30,39,29,18,25),
(8,4,13,31,41,30,18,25),
(8,4,14,31,42,31,18,26),
(8,4,15,32,44,31,19,26),
(8,4,16,33,45,32,19,27),
(8,4,17,34,46,33,19,27),
(8,4,18,35,48,34,19,27),
(8,4,19,36,50,34,19,28),
(8,4,20,36,51,35,20,28),
(8,4,21,37,53,36,20,29),
(8,4,22,38,54,37,20,29),
(8,4,23,39,56,38,20,30),
(8,4,24,40,57,38,21,30),
(8,4,25,41,59,39,21,31),
(8,4,26,42,61,40,21,31),
(8,4,27,43,62,41,21,31),
(8,4,28,44,64,42,21,32),
(8,4,29,44,66,43,22,32),
(8,4,30,45,68,43,22,33),
(8,4,31,46,69,44,22,33),
(8,4,32,47,71,45,22,34),
(8,4,33,48,73,46,23,34),
(8,4,34,49,75,47,23,35),
(8,4,35,50,77,48,23,35),
(8,4,36,52,79,49,24,36),
(8,4,37,53,80,50,24,37),
(8,4,38,54,82,51,24,37),
(8,4,39,55,84,52,24,38),
(8,4,40,56,86,53,25,38),
(8,4,41,57,88,54,25,39),
(8,4,42,58,90,55,25,39),
(8,4,43,59,92,56,25,40),
(8,4,44,60,95,57,26,40),
(8,4,45,62,97,58,26,41),
(8,4,46,63,99,59,26,42),
(8,4,47,64,101,60,27,42),
(8,4,48,65,103,61,27,43),
(8,4,49,66,105,63,27,44),
(8,4,50,68,108,64,28,44),
(8,4,51,69,110,65,28,45),
(8,4,52,70,112,66,28,45),
(8,4,53,71,115,67,29,46),
(8,4,54,73,117,68,29,47),
(8,4,55,74,119,70,29,47),
(8,4,56,75,122,71,30,48),
(8,4,57,77,124,72,30,49),
(8,4,58,78,127,73,30,50),
(8,4,59,80,129,75,31,50),
(8,4,60,81,132,76,31,51),
(8,5,1,21,22,21,18,24),
(8,5,2,21,22,21,19,25),
(8,5,3,21,22,22,20,26),
(8,5,4,22,23,22,21,28),
(8,5,5,22,23,22,23,29),
(8,5,6,22,23,23,24,30),
(8,5,7,22,23,23,25,31),
(8,5,8,22,24,23,26,32),
(8,5,9,22,24,24,27,34),
(8,5,10,23,24,24,29,35),
(8,5,11,23,24,25,30,36),
(8,5,12,23,25,25,31,38),
(8,5,13,23,25,25,32,39),
(8,5,14,23,25,26,34,40),
(8,5,15,24,25,26,35,42),
(8,5,16,24,26,27,36,43),
(8,5,17,24,26,27,38,44),
(8,5,18,24,26,27,39,46),
(8,5,19,24,27,28,40,47),
(8,5,20,25,27,28,42,49),
(8,5,21,25,27,29,43,50),
(8,5,22,25,27,29,45,52),
(8,5,23,25,28,30,46,53),
(8,5,24,26,28,30,48,55),
(8,5,25,26,28,31,49,56),
(8,5,26,26,29,31,51,58),
(8,5,27,26,29,31,52,60),
(8,5,28,26,29,32,54,61),
(8,5,29,27,30,32,55,63),
(8,5,30,27,30,33,57,65),
(8,5,31,27,30,33,59,66),
(8,5,32,27,31,34,60,68),
(8,5,33,28,31,34,62,70),
(8,5,34,28,31,35,64,71),
(8,5,35,28,32,35,65,73),
(8,5,36,29,32,36,67,75),
(8,5,37,29,32,37,69,77),
(8,5,38,29,33,37,71,79),
(8,5,39,29,33,38,72,81),
(8,5,40,30,33,38,74,82),
(8,5,41,30,34,39,76,84),
(8,5,42,30,34,39,78,86),
(8,5,43,30,35,40,80,88),
(8,5,44,31,35,40,82,90),
(8,5,45,31,35,41,84,92),
(8,5,46,31,36,42,86,94),
(8,5,47,32,36,42,88,96),
(8,5,48,32,37,43,90,99),
(8,5,49,32,37,44,92,101),
(8,5,50,33,37,44,94,103),
(8,5,51,33,38,45,96,105),
(8,5,52,33,38,45,98,107),
(8,5,53,34,39,46,100,109),
(8,5,54,34,39,47,102,112),
(8,5,55,34,40,47,105,114),
(8,5,56,35,40,48,107,116),
(8,5,57,35,41,49,109,119),
(8,5,58,35,41,50,111,121),
(8,5,59,36,42,50,114,124),
(8,5,60,36,42,51,116,126),
(8,7,1,22,22,22,17,23),
(8,7,2,23,22,23,18,24),
(8,7,3,23,23,24,19,25),
(8,7,4,24,23,25,19,26),
(8,7,5,25,24,25,20,27),
(8,7,6,26,24,26,21,28),
(8,7,7,27,24,27,22,29),
(8,7,8,27,25,28,23,29),
(8,7,9,28,25,29,24,30),
(8,7,10,29,26,30,24,31),
(8,7,11,30,26,31,25,32),
(8,7,12,31,27,32,26,33),
(8,7,13,31,27,33,27,34),
(8,7,14,32,28,34,28,35),
(8,7,15,33,28,35,29,37),
(8,7,16,34,29,36,30,38),
(8,7,17,35,29,37,31,39),
(8,7,18,36,30,38,32,40),
(8,7,19,37,30,39,33,41),
(8,7,20,38,31,40,34,42),
(8,7,21,39,31,41,35,43),
(8,7,22,39,32,42,36,44),
(8,7,23,40,32,43,37,45),
(8,7,24,41,33,44,38,47),
(8,7,25,42,33,46,39,48),
(8,7,26,43,34,47,40,49),
(8,7,27,44,34,48,41,50),
(8,7,28,45,35,49,42,51),
(8,7,29,46,35,50,43,53),
(8,7,30,47,36,51,44,54),
(8,7,31,49,36,53,46,55),
(8,7,32,50,37,54,47,57),
(8,7,33,51,38,55,48,58),
(8,7,34,52,38,56,49,59),
(8,7,35,53,39,58,50,61),
(8,7,36,54,40,59,52,62),
(8,7,37,55,40,60,53,63),
(8,7,38,56,41,62,54,65),
(8,7,39,57,41,63,55,66),
(8,7,40,59,42,64,57,68),
(8,7,41,60,43,66,58,69),
(8,7,42,61,43,67,59,71),
(8,7,43,62,44,69,60,72),
(8,7,44,64,45,70,62,74),
(8,7,45,65,45,72,63,75),
(8,7,46,66,46,73,65,77),
(8,7,47,67,47,75,66,78),
(8,7,48,69,48,76,67,80),
(8,7,49,70,48,78,69,82),
(8,7,50,71,49,79,70,83),
(8,7,51,73,50,81,72,85),
(8,7,52,74,51,82,73,87),
(8,7,53,76,51,84,75,88),
(8,7,54,77,52,86,76,90),
(8,7,55,78,53,87,78,92),
(8,7,56,80,54,89,79,94),
(8,7,57,81,55,91,81,95),
(8,7,58,83,55,92,83,97),
(8,7,59,84,56,94,84,99),
(8,7,60,86,57,96,86,101),
(8,8,1,21,22,21,19,23),
(8,8,2,21,22,21,20,24),
(8,8,3,21,22,22,21,25),
(8,8,4,21,23,22,23,26),
(8,8,5,21,23,22,24,28),
(8,8,6,22,23,22,25,29),
(8,8,7,22,23,23,26,30),
(8,8,8,22,23,23,27,31),
(8,8,9,22,23,23,29,32),
(8,8,10,22,24,24,30,34),
(8,8,11,22,24,24,31,35),
(8,8,12,22,24,24,33,36),
(8,8,13,22,24,25,34,37),
(8,8,14,23,24,25,35,39),
(8,8,15,23,25,25,37,40),
(8,8,16,23,25,26,38,41),
(8,8,17,23,25,26,39,43),
(8,8,18,23,25,26,41,44),
(8,8,19,23,25,27,42,45),
(8,8,20,23,26,27,44,47),
(8,8,21,24,26,27,45,48),
(8,8,22,24,26,28,47,50),
(8,8,23,24,26,28,48,51),
(8,8,24,24,27,29,50,53),
(8,8,25,24,27,29,51,54),
(8,8,26,24,27,29,53,56),
(8,8,27,24,27,30,55,57),
(8,8,28,25,27,30,56,59),
(8,8,29,25,28,31,58,60),
(8,8,30,25,28,31,60,62),
(8,8,31,25,28,31,61,64),
(8,8,32,25,28,32,63,65),
(8,8,33,25,29,32,65,67),
(8,8,34,26,29,33,66,69),
(8,8,35,26,29,33,68,70),
(8,8,36,26,30,34,70,72),
(8,8,37,26,30,34,72,74),
(8,8,38,26,30,34,74,76),
(8,8,39,27,30,35,76,77),
(8,8,40,27,31,35,77,79),
(8,8,41,27,31,36,79,81),
(8,8,42,27,31,36,81,83),
(8,8,43,27,31,37,83,85),
(8,8,44,27,32,37,85,87),
(8,8,45,28,32,38,87,89),
(8,8,46,28,32,38,89,91),
(8,8,47,28,33,39,91,93),
(8,8,48,28,33,39,94,95),
(8,8,49,29,33,40,96,97),
(8,8,50,29,34,40,98,99),
(8,8,51,29,34,41,100,101),
(8,8,52,29,34,41,102,103),
(8,8,53,29,35,42,104,105),
(8,8,54,30,35,43,107,107),
(8,8,55,30,35,43,109,110),
(8,8,56,30,36,44,111,112),
(8,8,57,30,36,44,114,114),
(8,8,58,31,36,45,116,116),
(8,8,59,31,37,45,119,119),
(8,8,60,31,37,46,121,121);
/*!40000 ALTER TABLE `player_levelstats` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_xp_for_level`
--

DROP TABLE IF EXISTS `player_xp_for_level`;
CREATE TABLE `player_xp_for_level` (
  `lvl` int(3) unsigned NOT NULL,
  `xp_for_next_level` int(10) unsigned NOT NULL,
  PRIMARY KEY (`lvl`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `player_xp_for_level`
--

LOCK TABLES `player_xp_for_level` WRITE;
/*!40000 ALTER TABLE `player_xp_for_level` DISABLE KEYS */;
INSERT INTO `player_xp_for_level` VALUES
(1,400),
(2,900),
(3,1400),
(4,2100),
(5,2800),
(6,3600),
(7,4500),
(8,5400),
(9,6500),
(10,7600),
(11,8800),
(12,10100),
(13,11400),
(14,12900),
(15,14400),
(16,16000),
(17,17700),
(18,19400),
(19,21300),
(20,23200),
(21,25200),
(22,27300),
(23,29400),
(24,31700),
(25,34000),
(26,36400),
(27,38900),
(28,41400),
(29,44300),
(30,47400),
(31,50800),
(32,54500),
(33,58600),
(34,62800),
(35,67100),
(36,71600),
(37,76100),
(38,80800),
(39,85700),
(40,90700),
(41,95800),
(42,101000),
(43,106300),
(44,111800),
(45,117500),
(46,123200),
(47,129100),
(48,135100),
(49,141200),
(50,147500),
(51,153900),
(52,160400),
(53,167100),
(54,173900),
(55,180800),
(56,187900),
(57,195000),
(58,202300),
(59,209800);
/*!40000 ALTER TABLE `player_xp_for_level` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `playercreateinfo`
--

DROP TABLE IF EXISTS `playercreateinfo`;
CREATE TABLE `playercreateinfo` (
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `zone` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo`
--

LOCK TABLES `playercreateinfo` WRITE;
/*!40000 ALTER TABLE `playercreateinfo` DISABLE KEYS */;
INSERT INTO `playercreateinfo` VALUES
(1,1,0,12,-8949.95,-132.493,83.5312,0),
(1,2,0,12,-8949.95,-132.493,83.5312,0),
(1,4,0,12,-8949.95,-132.493,83.5312,0),
(1,5,0,12,-8949.95,-132.493,83.5312,0),
(1,8,0,12,-8949.95,-132.493,83.5312,0),
(1,9,0,12,-8949.95,-132.493,83.5312,0),
(2,1,1,14,-618.518,-4251.67,38.718,0),
(2,3,1,14,-618.518,-4251.67,38.718,0),
(2,4,1,14,-618.518,-4251.67,38.718,0),
(2,7,1,14,-618.518,-4251.67,38.718,0),
(2,9,1,14,-618.518,-4251.67,38.718,0),
(3,1,0,1,-6240.32,331.033,382.758,6.17716),
(3,2,0,1,-6240.32,331.033,382.758,6.17716),
(3,3,0,1,-6240.32,331.033,382.758,6.17716),
(3,4,0,1,-6240.32,331.033,382.758,6.17716),
(3,5,0,1,-6240.32,331.033,382.758,6.17716),
(4,1,1,141,10311.3,832.463,1326.41,5.69632),
(4,3,1,141,10311.3,832.463,1326.41,5.69632),
(4,4,1,141,10311.3,832.463,1326.41,5.69632),
(4,5,1,141,10311.3,832.463,1326.41,5.69632),
(4,11,1,141,10311.3,832.463,1326.41,5.69632),
(5,1,0,85,1676.71,1678.31,121.67,2.70526),
(5,4,0,85,1676.71,1678.31,121.67,2.70526),
(5,5,0,85,1676.71,1678.31,121.67,2.70526),
(5,8,0,85,1676.71,1678.31,121.67,2.70526),
(5,9,0,85,1676.71,1678.31,121.67,2.70526),
(6,1,1,215,-2917.58,-257.98,52.9968,0),
(6,3,1,215,-2917.58,-257.98,52.9968,0),
(6,7,1,215,-2917.58,-257.98,52.9968,0),
(6,11,1,215,-2917.58,-257.98,52.9968,0),
(7,1,0,1,-6240.32,331.033,382.758,0),
(7,4,0,1,-6240.32,331.033,382.758,0),
(7,8,0,1,-6240.32,331.033,382.758,0),
(7,9,0,1,-6240.32,331.033,382.758,0),
(8,1,1,14,-618.518,-4251.67,38.718,0),
(8,3,1,14,-618.518,-4251.67,38.718,0),
(8,4,1,14,-618.518,-4251.67,38.718,0),
(8,5,1,14,-618.518,-4251.67,38.718,0),
(8,7,1,14,-618.518,-4251.67,38.718,0),
(8,8,1,14,-618.518,-4251.67,38.718,0);
/*!40000 ALTER TABLE `playercreateinfo` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `playercreateinfo_action`
--

DROP TABLE IF EXISTS `playercreateinfo_action`;
CREATE TABLE `playercreateinfo_action` (
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `button` smallint(5) unsigned NOT NULL DEFAULT '0',
  `action` int(11) unsigned NOT NULL DEFAULT '0',
  `type` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`race`,`class`,`button`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_action`
--

LOCK TABLES `playercreateinfo_action` WRITE;
/*!40000 ALTER TABLE `playercreateinfo_action` DISABLE KEYS */;
INSERT INTO `playercreateinfo_action` VALUES
(8,8,11,117,128),
(8,8,10,159,128),
(8,8,2,168,0),
(8,8,1,133,0),
(8,8,0,6603,0),
(8,7,11,117,128),
(8,7,10,159,128),
(8,7,3,20544,0),
(8,7,2,331,0),
(8,7,1,403,0),
(8,7,0,6603,0),
(8,5,11,4540,128),
(8,5,2,2050,0),
(8,5,1,585,0),
(8,5,0,6603,0),
(8,4,11,117,128),
(8,4,4,26972,0),
(8,4,2,2098,0),
(8,4,1,1752,0),
(8,4,0,6603,0),
(8,3,11,4604,128),
(8,3,10,159,128),
(8,3,2,75,0),
(8,3,1,2973,0),
(8,3,0,6603,0),
(8,1,83,117,128),
(8,1,75,26296,0),
(8,1,73,78,0),
(8,1,72,6603,0),
(7,9,11,4604,128),
(7,9,10,159,128),
(7,9,2,687,0),
(7,9,0,6603,0),
(7,8,11,4536,128),
(7,8,10,159,128),
(7,8,2,168,0),
(7,8,1,133,0),
(7,8,0,6603,0),
(7,4,11,117,128),
(7,4,3,2764,0),
(7,4,2,2098,0),
(7,4,1,1752,0),
(7,4,0,6603,0),
(7,1,83,117,128),
(7,1,73,78,0),
(7,1,72,6603,0),
(6,11,11,4536,128),
(6,11,10,159,128),
(6,11,3,20549,0),
(6,11,2,5185,0),
(6,11,1,5176,0),
(6,11,0,6603,0),
(6,7,11,4604,128),
(6,7,10,159,128),
(6,7,3,20549,0),
(6,7,2,331,0),
(6,7,1,403,0),
(6,7,0,6603,0),
(6,3,11,117,128),
(6,3,10,159,128),
(6,3,3,20549,0),
(6,3,2,75,0),
(6,3,1,2973,0),
(6,3,0,6603,0),
(6,1,83,4540,128),
(6,1,74,20549,0),
(6,1,73,78,0),
(6,1,72,6603,0),
(5,9,11,4604,128),
(5,9,10,159,128),
(5,9,3,20577,0),
(5,9,2,687,0),
(5,9,1,686,0),
(5,9,0,6603,0),
(5,8,11,4604,128),
(5,8,10,159,128),
(5,8,3,20577,0),
(5,8,2,168,0),
(5,8,1,133,0),
(5,8,0,6603,0),
(5,5,11,4604,128),
(5,5,10,159,128),
(5,5,3,20577,0),
(5,5,2,2050,0),
(5,5,1,585,0),
(5,5,0,6603,0),
(5,4,11,4604,128),
(5,4,4,20577,0),
(5,4,3,2764,0),
(5,4,2,2098,0),
(5,4,1,1752,0),
(5,4,0,6603,0),
(5,1,83,4604,128),
(5,1,74,20577,0),
(5,1,73,78,0),
(5,1,72,6603,0),
(4,11,11,4536,128),
(8,5,10,159,128),
(4,11,10,159,128),
(4,11,2,5185,0),
(4,11,1,5176,0),
(4,11,0,6603,0),
(4,5,11,2070,128),
(4,5,10,159,128),
(4,5,3,20580,0),
(4,5,2,2050,0),
(4,5,1,585,0),
(4,5,0,6603,0),
(4,4,11,4540,128),
(4,4,3,2764,0),
(4,4,2,2098,0),
(4,4,1,1752,0),
(4,4,0,6603,0),
(4,3,11,117,128),
(4,3,10,159,128),
(4,3,3,20580,0),
(4,3,2,75,0),
(4,3,1,2973,0),
(4,3,0,6603,0),
(4,1,83,117,128),
(4,1,74,20580,0),
(4,1,73,78,0),
(4,1,72,6603,0),
(3,5,11,4540,128),
(8,4,3,2764,0),
(8,3,3,20544,0),
(8,1,74,2764,0),
(7,9,1,686,0),
(3,5,10,159,128),
(3,5,4,2481,0),
(3,5,3,20594,0),
(3,5,2,2050,0),
(3,5,1,585,0),
(3,5,0,6603,0),
(3,4,11,4540,128),
(3,4,5,2481,0),
(3,4,4,20594,0),
(3,4,3,2764,0),
(3,4,2,2098,0),
(3,4,1,1752,0),
(3,4,0,6603,0),
(3,3,11,117,128),
(3,3,10,159,128),
(3,3,4,2481,0),
(3,3,3,20594,0),
(3,3,2,75,0),
(3,3,1,2973,0),
(3,3,0,6603,0),
(3,2,11,4540,128),
(3,2,10,159,128),
(3,2,4,2481,0),
(3,2,3,20594,0),
(3,2,2,635,0),
(3,2,1,20154,0),
(3,2,0,6603,0),
(3,1,83,117,128),
(3,1,75,2481,0),
(3,1,74,20594,0),
(3,1,73,78,0),
(3,1,72,6603,0),
(2,9,11,117,128),
(2,9,10,159,128),
(2,9,3,20572,0),
(2,9,2,687,0),
(2,9,1,686,0),
(2,9,0,6603,0),
(2,7,11,117,128),
(2,7,10,159,128),
(2,7,3,20572,0),
(2,7,2,331,0),
(2,7,1,403,0),
(2,7,0,6603,0),
(2,4,11,117,128),
(2,4,4,20572,0),
(2,4,3,2764,0),
(2,4,2,2098,0),
(2,4,1,1752,0),
(2,4,0,6603,0),
(2,3,11,117,128),
(2,3,10,159,128),
(2,3,4,20572,0),
(2,3,2,75,0),
(2,3,1,2973,0),
(2,3,0,6603,0),
(2,1,83,117,128),
(2,1,74,20572,0),
(2,1,73,78,0),
(2,1,72,6603,0),
(1,9,11,4604,128),
(1,9,10,159,128),
(1,9,2,687,0),
(1,9,1,686,0),
(1,9,0,6603,0),
(1,8,11,2070,128),
(1,8,10,159,128),
(1,8,2,168,0),
(1,8,1,133,0),
(1,8,0,6603,0),
(1,5,11,2070,128),
(1,5,10,159,128),
(1,5,2,2050,0),
(1,5,1,585,0),
(1,5,0,6603,0),
(1,4,11,2070,128),
(1,4,3,2764,0),
(1,4,2,2098,0),
(1,4,1,1752,0),
(1,4,0,6603,0),
(1,2,11,2070,128),
(1,2,10,159,128),
(1,2,2,635,0),
(1,2,1,20154,0),
(1,2,0,6603,0),
(1,1,83,117,128),
(1,1,73,78,0),
(1,1,72,6603,0),
(4,11,3,20580,0);
/*!40000 ALTER TABLE `playercreateinfo_action` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `playercreateinfo_item`
--

DROP TABLE IF EXISTS `playercreateinfo_item`;
CREATE TABLE `playercreateinfo_item` (
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `amount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  KEY `playercreateinfo_race_class_index` (`race`,`class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_item`
--

LOCK TABLES `playercreateinfo_item` WRITE;
/*!40000 ALTER TABLE `playercreateinfo_item` DISABLE KEYS */;
/*!40000 ALTER TABLE `playercreateinfo_item` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `playercreateinfo_skills`
--

DROP TABLE IF EXISTS `playercreateinfo_skills`;
CREATE TABLE `playercreateinfo_skills` (
  `raceMask` int unsigned NOT NULL,
  `classMask` int unsigned NOT NULL,
  `skill` smallint(5) unsigned NOT NULL,
  `step` smallint(5) unsigned NOT NULL DEFAULT '0',
  `note` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`raceMask`,`classMask`,`skill`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_skills`
--

LOCK TABLES `playercreateinfo_skills` WRITE;
/*!40000 ALTER TABLE `playercreateinfo_skills` DISABLE KEYS */;
INSERT INTO `playercreateinfo_skills` VALUES
-- ALL PLAYERS:
(0,     0,  95, 0,  'Misc: Defense'),
(0,     0, 162, 0,  'Weapon: Unarmed'),
(0,     0, 183, 0,  'Misc: GENERIC (DND)'),
(0,     0, 415, 0,  'Armor: Cloth'),
-- WARRIOR CLASS:
(0,     1,  26, 0,  'Warrior: Arms'),
(0,     1, 256, 0,  'Warrior: Fury'),
(0,     1, 257, 0,  'Warrior: Protection'),
(167,   1,  44, 0,  'Weapon: Axes (Warrior)'),
(216,   1, 173, 0,  'Weapon: Daggers (Warrior)'),
(109,   1,  54, 0,  'Weapon: Maces (Warrior)'),
(91,    1,  43, 0,  'Weapon: Swords (Warrior)'),
(128,   1, 176, 0,  'Weapon: Thrown (Warrior)'),
(6,     1, 172, 0,  'Weapon: Two-Handed Axes (Warrior)'),
(32,    1, 160, 0,  'Weapon: Two-Handed Maces (Warrior)'),
(16,    1,  55, 0,  'Weapon: Two-Handed Swords (Warrior)'),
-- PALADIN CLASS:
(0,     2, 594, 0,  'Paladin: Holy'),
(0,     2, 267, 0,  'Paladin: Protection'),
(0,     2, 184, 0,  'Paladin: Retribution'),
(0,     2,  54, 0,  'Weapon: Maces (Paladin)'),
(0,     2, 160, 0,  'Weapon: Two-Handed Maces (Paladin)'),
-- HUNTER CLASS:
(0,     4,  50, 0,  'Hunter: Beast Mastery'),
(0,     4, 163, 0,  'Hunter: Marksmanship'),
(0,     4,  51, 0,  'Hunter: Survival'),
(166,   4,  44, 0,  'Weapon: Axes (Hunter)'),
(138,   4,  45, 0,  'Weapon: Bows (Hunter)'),
(8,     4, 173, 0,  'Weapon: Daggers (Hunter)'),
(36,    4,  46, 0,  'Weapon: Guns (Hunter)'),
-- ROGUE CLASS:
(0,     8,  38, 0,  'Rogue: Combat'),
(0,     8, 253, 0,  'Rogue: Assassination'),
(0,     8,  39, 0,  'Rogue: Subtlety'),
(0,     8, 173, 0,  'Weapon: Daggers (Rogue)'),
(0,     8, 176, 0,  'Weapon: Thrown (Rogue)'),
-- PRIEST CLASS:
(0,    16,  56, 0,  'Priest: Holy'),
(0,    16, 613, 0,  'Priest: Discipline'),
(0,    16,  78, 0,  'Priest: Shadow'),
(0,    16,  54, 0,  'Weapon: Maces (Priest)'),
(0,    16, 228, 0,  'Weapon: Wands (Priest)'),
-- SHAMAN CLASS:
(0,    64, 375, 0,  'Shaman: Elemental'),
(0,    64, 373, 0,  'Shaman: Enhancement'),
(0,    64, 374, 0,  'Shaman: Restoration'),
(0,    64,  54, 0,  'Weapon: Maces (Shaman)'),
(0,    64, 136, 0,  'Weapon: Staves (Shaman)'),
-- MAGE CLASS:
(0,   128, 237, 0,  'Mage: Arcane'),
(0,   128,   8, 0,  'Mage: Fire'),
(0,   128,   6, 0,  'Mage: Frost'),
(0,   128, 136, 0,  'Weapon: Staves (Mage)'),
(0,   128, 228, 0,  'Weapon: Wands (Mage)'),
-- WARLOCK CLASS:
(0,   256, 355, 0,  'Warlock: Affliction'),
(0,   256, 354, 0,  'Warlock: Demonology'),
(0,   256, 593, 0,  'Warlock: Destruction'),
(0,   256, 173, 0,  'Weapon: Daggers (Warlock)'),
(0,   256, 228, 0,  'Weapon: Wands (Warlock)'),
-- DRUID CLASS:
(0,  1024, 574, 0,  'Druid: Balance'),
(0,  1024, 134, 0,  'Druid: Feral Combat'),
(0,  1024, 573, 0,  'Druid: Restoration'),
(8,  1024, 173, 0,  'Weapon: Daggers (Druid)'),
(32, 1024,  54, 0,  'Weapon: Maces (Druid)'),
(0,  1024, 136, 0,  'Weapon: Staves (Druid)'),
-- ARMOR AND MISC SKILLS:
(0,     3, 413, 0,  'Armor: Mail'),
(0,  1103, 414, 0,  'Armor: Leather'),
(0,    67, 433, 0,  'Armor: Shield'),
-- ALLIANCE RACES:
(1,     0, 754, 0,  'Racial: Human'),
(77,    0,  98, 0,  'Language: Common'),
(4,     0, 101, 0,  'Racial: Dwarf'),
(4,     0, 111, 0,  'Language: Dwarven'),
(8,     0, 126, 0,  'Racial: Night Elf'),
(8,     0, 113, 0,  'Language: Darnassian'),
(64,    0, 753, 0,  'Racial: Gnome'),
(64,    0, 313, 0,  'Language: Gnomish'),
-- HORDE RACES:
(2,     0, 125, 0,  'Racial: Orc'),
(178,   0, 109, 0,  'Language: Orcish'),
(16,    0, 220, 0,  'Racial: Undead'),
(16,    0, 673, 0,  'Language: Gutterspeak'),
(32,    0, 124, 0,  'Racial: Tauren'),
(32,    0, 115, 0,  'Language: Taurahe'),
(128,   0, 733, 0,  'Racial: Troll'),
(128,   0, 315, 0,  'Language: Troll');
/*!40000 ALTER TABLE `playercreateinfo_skills` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `playercreateinfo_spell`
--

DROP TABLE IF EXISTS `playercreateinfo_spell`;
CREATE TABLE `playercreateinfo_spell` (
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Note` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`race`,`class`,`Spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `playercreateinfo_spell`
--

LOCK TABLES `playercreateinfo_spell` WRITE;
/*!40000 ALTER TABLE `playercreateinfo_spell` DISABLE KEYS */;
INSERT INTO `playercreateinfo_spell` VALUES
(1,1,78,'Heroic Strike'),
(1,1,81,'Dodge'),
(1,1,107,'Block'),
(1,1,196,'One-Handed Axes'),
(1,1,198,'One-Handed Maces'),
(1,1,201,'One-Handed Swords'),
(1,1,203,'Unarmed'),
(1,1,204,'Defense'),
(1,1,522,'SPELLDEFENSE (DND)'),
(1,1,668,'Language Common'),
(1,1,2382,'Generic'),
(1,1,2457,'Battle Stance'),
(1,1,2479,'Honorless Target'),
(1,1,3050,'Detect'),
(1,1,3365,'Opening'),
(1,1,5301,'Defensive State (DND)'),
(1,1,6233,'Closing'),
(1,1,6246,'Closing'),
(1,1,6247,'Opening'),
(1,1,6477,'Opening'),
(1,1,6478,'Opening'),
(1,1,6603,'Attack'),
(1,1,7266,'Duel'),
(1,1,7267,'Grovel'),
(1,1,7355,'Stuck'),
(1,1,8386,'Attacking'),
(1,1,8737,'Mail'),
(1,1,9077,'Leather'),
(1,1,9078,'Cloth'),
(1,1,9116,'Shield'),
(1,1,9125,'Generic'),
(1,1,20597,'Sword Specialization'),
(1,1,20598,'The Human Spirit'),
(1,1,20599,'Diplomacy'),
(1,1,20600,'Perception'),
(1,1,20864,'Mace Specialization'),
(1,1,21651,'Opening'),
(1,1,21652,'Closing'),
(1,1,22027,'Remove Insignia'),
(1,1,22810,'Opening - No Text'),
(1,2,81,'Dodge'),
(1,2,107,'Block'),
(1,2,198,'One-Handed Maces'),
(1,2,199,'Two-Handed Maces'),
(1,2,203,'Unarmed'),
(1,2,204,'Defense'),
(1,2,522,'SPELLDEFENSE (DND)'),
(1,2,635,'Holy Light'),
(1,2,668,'Language Common'),
(1,2,2382,'Generic'),
(1,2,2479,'Honorless Target'),
(1,2,3050,'Detect'),
(1,2,3365,'Opening'),
(1,2,6233,'Closing'),
(1,2,6246,'Closing'),
(1,2,6247,'Opening'),
(1,2,6477,'Opening'),
(1,2,6478,'Opening'),
(1,2,6603,'Attack'),
(1,2,7266,'Duel'),
(1,2,7267,'Grovel'),
(1,2,7355,'Stuck'),
(1,2,8386,'Attacking'),
(1,2,8737,'Mail'),
(1,2,9077,'Leather'),
(1,2,9078,'Cloth'),
(1,2,9116,'Shield'),
(1,2,9125,'Generic'),
(1,2,20154,'Seal of Righteousness'),
(1,2,20597,'Sword Specialization'),
(1,2,20598,'The Human Spirit'),
(1,2,20599,'Diplomacy'),
(1,2,20600,'Perception'),
(1,2,20864,'Mace Specialization'),
(1,2,21651,'Opening'),
(1,2,21652,'Closing'),
(1,2,22027,'Remove Insignia'),
(1,2,22810,'Opening - No Text'),
(1,2,27762,'Libram'),
(1,4,81,'Dodge'),
(1,4,203,'Unarmed'),
(1,4,204,'Defense'),
(1,4,522,'SPELLDEFENSE (DND)'),
(1,4,668,'Language Common'),
(1,4,1180,'Daggers'),
(1,4,1752,'Sinister Strike'),
(1,4,2098,'Eviscerate'),
(1,4,2382,'Generic'),
(1,4,2479,'Honorless Target'),
(1,4,2567,'Thrown'),
(1,4,2764,'Throw'),
(1,4,3050,'Detect'),
(1,4,3365,'Opening'),
(1,4,6233,'Closing'),
(1,4,6246,'Closing'),
(1,4,6247,'Opening'),
(1,4,6477,'Opening'),
(1,4,6478,'Opening'),
(1,4,6603,'Attack'),
(1,4,7266,'Duel'),
(1,4,7267,'Grovel'),
(1,4,7355,'Stuck'),
(1,4,8386,'Attacking'),
(1,4,9077,'Leather'),
(1,4,9078,'Cloth'),
(1,4,9125,'Generic'),
(1,4,16092,'Defensive State (DND)'),
(1,4,20597,'Sword Specialization'),
(1,4,20598,'The Human Spirit'),
(1,4,20599,'Diplomacy'),
(1,4,20600,'Perception'),
(1,4,20864,'Mace Specialization'),
(1,4,21184,'Rogue Passive (DND)'),
(1,4,21651,'Opening'),
(1,4,21652,'Closing'),
(1,4,22027,'Remove Insignia'),
(1,4,22810,'Opening - No Text'),
(1,5,81,'Dodge'),
(1,5,198,'One-Handed Maces'),
(1,5,203,'Unarmed'),
(1,5,204,'Defense'),
(1,5,522,'SPELLDEFENSE (DND)'),
(1,5,585,'Smite'),
(1,5,668,'Language Common'),
(1,5,2050,'Lesser Heal'),
(1,5,2382,'Generic'),
(1,5,2479,'Honorless Target'),
(1,5,3050,'Detect'),
(1,5,3365,'Opening'),
(1,5,5009,'Wands'),
(1,5,5019,'Shoot'),
(1,5,6233,'Closing'),
(1,5,6246,'Closing'),
(1,5,6247,'Opening'),
(1,5,6477,'Opening'),
(1,5,6478,'Opening'),
(1,5,6603,'Attack'),
(1,5,7266,'Duel'),
(1,5,7267,'Grovel'),
(1,5,7355,'Stuck'),
(1,5,8386,'Attacking'),
(1,5,9078,'Cloth'),
(1,5,9125,'Generic'),
(1,5,20597,'Sword Specialization'),
(1,5,20598,'The Human Spirit'),
(1,5,20599,'Diplomacy'),
(1,5,20600,'Perception'),
(1,5,20864,'Mace Specialization'),
(1,5,21651,'Opening'),
(1,5,21652,'Closing'),
(1,5,22027,'Remove Insignia'),
(1,5,22810,'Opening - No Text'),
(1,8,81,'Dodge'),
(1,8,133,'Fireball'),
(1,8,168,'Frost Armor'),
(1,8,203,'Unarmed'),
(1,8,204,'Defense'),
(1,8,227,'Staves'),
(1,8,522,'SPELLDEFENSE (DND)'),
(1,8,668,'Language Common'),
(1,8,2382,'Generic'),
(1,8,2479,'Honorless Target'),
(1,8,3050,'Detect'),
(1,8,3365,'Opening'),
(1,8,5009,'Wands'),
(1,8,5019,'Shoot'),
(1,8,6233,'Closing'),
(1,8,6246,'Closing'),
(1,8,6247,'Opening'),
(1,8,6477,'Opening'),
(1,8,6478,'Opening'),
(1,8,6603,'Attack'),
(1,8,7266,'Duel'),
(1,8,7267,'Grovel'),
(1,8,7355,'Stuck'),
(1,8,8386,'Attacking'),
(1,8,9078,'Cloth'),
(1,8,9125,'Generic'),
(1,8,20597,'Sword Specialization'),
(1,8,20598,'The Human Spirit'),
(1,8,20599,'Diplomacy'),
(1,8,20600,'Perception'),
(1,8,20864,'Mace Specialization'),
(1,8,21651,'Opening'),
(1,8,21652,'Closing'),
(1,8,22027,'Remove Insignia'),
(1,8,22810,'Opening - No Text'),
(1,9,81,'Dodge'),
(1,9,203,'Unarmed'),
(1,9,204,'Defense'),
(1,9,522,'SPELLDEFENSE (DND)'),
(1,9,668,'Language Common'),
(1,9,686,'Shadow Bolt'),
(1,9,687,'Demon Skin'),
(1,9,1180,'Daggers'),
(1,9,2382,'Generic'),
(1,9,2479,'Honorless Target'),
(1,9,3050,'Detect'),
(1,9,3365,'Opening'),
(1,9,5009,'Wands'),
(1,9,5019,'Shoot'),
(1,9,6233,'Closing'),
(1,9,6246,'Closing'),
(1,9,6247,'Opening'),
(1,9,6477,'Opening'),
(1,9,6478,'Opening'),
(1,9,6603,'Attack'),
(1,9,7266,'Duel'),
(1,9,7267,'Grovel'),
(1,9,7355,'Stuck'),
(1,9,8386,'Attacking'),
(1,9,9078,'Cloth'),
(1,9,9125,'Generic'),
(1,9,20597,'Sword Specialization'),
(1,9,20598,'The Human Spirit'),
(1,9,20599,'Diplomacy'),
(1,9,20600,'Perception'),
(1,9,20864,'Mace Specialization'),
(1,9,21651,'Opening'),
(1,9,21652,'Closing'),
(1,9,22027,'Remove Insignia'),
(1,9,22810,'Opening - No Text'),
(2,1,78,'Heroic Strike'),
(2,1,81,'Dodge'),
(2,1,107,'Block'),
(2,1,196,'One-Handed Axes'),
(2,1,197,'Two-Handed Axes'),
(2,1,201,'One-Handed Swords'),
(2,1,203,'Unarmed'),
(2,1,204,'Defense'),
(2,1,522,'SPELLDEFENSE (DND)'),
(2,1,669,'Language Orcish'),
(2,1,2382,'Generic'),
(2,1,2457,'Battle Stance'),
(2,1,2479,'Honorless Target'),
(2,1,3050,'Detect'),
(2,1,3365,'Opening'),
(2,1,5301,'Defensive State (DND)'),
(2,1,6233,'Closing'),
(2,1,6246,'Closing'),
(2,1,6247,'Opening'),
(2,1,6477,'Opening'),
(2,1,6478,'Opening'),
(2,1,6603,'Attack'),
(2,1,7266,'Duel'),
(2,1,7267,'Grovel'),
(2,1,7355,'Stuck'),
(2,1,8386,'Attacking'),
(2,1,8737,'Mail'),
(2,1,9077,'Leather'),
(2,1,9078,'Cloth'),
(2,1,9116,'Shield'),
(2,1,9125,'Generic'),
(2,1,20572,'Blood Fury'),
(2,1,20573,'Hardiness'),
(2,1,20574,'Axe Specialization'),
(2,1,21563,'Command'),
(2,1,21651,'Opening'),
(2,1,21652,'Closing'),
(2,1,22027,'Remove Insignia'),
(2,1,22810,'Opening - No Text'),
(2,3,75,'Auto Shot'),
(2,3,81,'Dodge'),
(2,3,196,'One-Handed Axes'),
(2,3,203,'Unarmed'),
(2,3,204,'Defense'),
(2,3,264,'Bows'),
(2,3,522,'SPELLDEFENSE (DND)'),
(2,3,669,'Language Orcish'),
(2,3,2382,'Generic'),
(2,3,2479,'Honorless Target'),
(2,3,2973,'Raptor Strike'),
(2,3,3050,'Detect'),
(2,3,3365,'Opening'),
(2,3,6233,'Closing'),
(2,3,6246,'Closing'),
(2,3,6247,'Opening'),
(2,3,6477,'Opening'),
(2,3,6478,'Opening'),
(2,3,6603,'Attack'),
(2,3,7266,'Duel'),
(2,3,7267,'Grovel'),
(2,3,7355,'Stuck'),
(2,3,8386,'Attacking'),
(2,3,9077,'Leather'),
(2,3,9078,'Cloth'),
(2,3,9125,'Generic'),
(2,3,13358,'Defensive State (DND)'),
(2,3,20572,'Blood Fury'),
(2,3,20573,'Hardiness'),
(2,3,20574,'Axe Specialization'),
(2,3,20576,'Command'),
(2,3,21651,'Opening'),
(2,3,21652,'Closing'),
(2,3,22027,'Remove Insignia'),
(2,3,22810,'Opening - No Text'),
(2,3,24949,'Defensive State 2 (DND)'),
(2,4,81,'Dodge'),
(2,4,203,'Unarmed'),
(2,4,204,'Defense'),
(2,4,522,'SPELLDEFENSE (DND)'),
(2,4,669,'Language Orcish'),
(2,4,1180,'Daggers'),
(2,4,1752,'Sinister Strike'),
(2,4,2098,'Eviscerate'),
(2,4,2382,'Generic'),
(2,4,2479,'Honorless Target'),
(2,4,2567,'Thrown'),
(2,4,2764,'Throw'),
(2,4,3050,'Detect'),
(2,4,3365,'Opening'),
(2,4,6233,'Closing'),
(2,4,6246,'Closing'),
(2,4,6247,'Opening'),
(2,4,6477,'Opening'),
(2,4,6478,'Opening'),
(2,4,6603,'Attack'),
(2,4,7266,'Duel'),
(2,4,7267,'Grovel'),
(2,4,7355,'Stuck'),
(2,4,8386,'Attacking'),
(2,4,9077,'Leather'),
(2,4,9078,'Cloth'),
(2,4,9125,'Generic'),
(2,4,16092,'Defensive State (DND)'),
(2,4,20572,'Blood Fury'),
(2,4,20573,'Hardiness'),
(2,4,20574,'Axe Specialization'),
(2,4,21184,'Rogue Passive (DND)'),
(2,4,21563,'Command'),
(2,4,21651,'Opening'),
(2,4,21652,'Closing'),
(2,4,22027,'Remove Insignia'),
(2,4,22810,'Opening - No Text'),
(2,7,81,'Dodge'),
(2,7,107,'Block'),
(2,7,198,'One-Handed Maces'),
(2,7,203,'Unarmed'),
(2,7,204,'Defense'),
(2,7,227,'Staves'),
(2,7,331,'Healing Wave'),
(2,7,403,'Lightning Bolt'),
(2,7,522,'SPELLDEFENSE (DND)'),
(2,7,669,'Language Orcish'),
(2,7,2382,'Generic'),
(2,7,2479,'Honorless Target'),
(2,7,3050,'Detect'),
(2,7,3365,'Opening'),
(2,7,6233,'Closing'),
(2,7,6246,'Closing'),
(2,7,6247,'Opening'),
(2,7,6477,'Opening'),
(2,7,6478,'Opening'),
(2,7,6603,'Attack'),
(2,7,7266,'Duel'),
(2,7,7267,'Grovel'),
(2,7,7355,'Stuck'),
(2,7,8386,'Attacking'),
(2,7,9077,'Leather'),
(2,7,9078,'Cloth'),
(2,7,9116,'Shield'),
(2,7,9125,'Generic'),
(2,7,20573,'Hardiness'),
(2,7,20574,'Axe Specialization'),
(2,7,21651,'Opening'),
(2,7,21652,'Closing'),
(2,7,22027,'Remove Insignia'),
(2,7,22810,'Opening - No Text'),
(2,7,27763,'Totem'),
(2,7,20572,'Blood Fury'),
(2,9,81,'Dodge'),
(2,9,203,'Unarmed'),
(2,9,204,'Defense'),
(2,9,522,'SPELLDEFENSE (DND)'),
(2,9,669,'Language Orcish'),
(2,9,686,'Shadow Bolt'),
(2,9,687,'Demon Skin'),
(2,9,1180,'Daggers'),
(2,9,2382,'Generic'),
(2,9,2479,'Honorless Target'),
(2,9,3050,'Detect'),
(2,9,3365,'Opening'),
(2,9,5009,'Wands'),
(2,9,5019,'Shoot'),
(2,9,6233,'Closing'),
(2,9,6246,'Closing'),
(2,9,6247,'Opening'),
(2,9,6477,'Opening'),
(2,9,6478,'Opening'),
(2,9,6603,'Attack'),
(2,9,7266,'Duel'),
(2,9,7267,'Grovel'),
(2,9,7355,'Stuck'),
(2,9,8386,'Attacking'),
(2,9,9078,'Cloth'),
(2,9,9125,'Generic'),
(2,9,20573,'Hardiness'),
(2,9,20574,'Axe Specialization'),
(2,9,20575,'Command'),
(2,9,21651,'Opening'),
(2,9,21652,'Closing'),
(2,9,22027,'Remove Insignia'),
(2,9,22810,'Opening - No Text'),
(2,9,20572,'Blood Fury'),
(3,1,78,'Heroic Strike'),
(3,1,81,'Dodge'),
(3,1,107,'Block'),
(3,1,196,'One-Handed Axes'),
(3,1,197,'Two-Handed Axes'),
(3,1,198,'One-Handed Maces'),
(3,1,203,'Unarmed'),
(3,1,204,'Defense'),
(3,1,522,'SPELLDEFENSE (DND)'),
(3,1,668,'Language Common'),
(3,1,672,'Language Dwarven'),
(3,1,2382,'Generic'),
(3,1,2457,'Battle Stance'),
(3,1,2479,'Honorless Target'),
(3,1,2481,'Find Treasure'),
(3,1,3050,'Detect'),
(3,1,3365,'Opening'),
(3,1,5301,'Defensive State (DND)'),
(3,1,6233,'Closing'),
(3,1,6246,'Closing'),
(3,1,6247,'Opening'),
(3,1,6477,'Opening'),
(3,1,6478,'Opening'),
(3,1,6603,'Attack'),
(3,1,7266,'Duel'),
(3,1,7267,'Grovel'),
(3,1,7355,'Stuck'),
(3,1,8386,'Attacking'),
(3,1,8737,'Mail'),
(3,1,9077,'Leather'),
(3,1,9078,'Cloth'),
(3,1,9116,'Shield'),
(3,1,9125,'Generic'),
(3,1,20594,'Stoneform'),
(3,1,20595,'Gun Specialization'),
(3,1,20596,'Frost Resistance'),
(3,1,21651,'Opening'),
(3,1,21652,'Closing'),
(3,1,22027,'Remove Insignia'),
(3,1,22810,'Opening - No Text'),
(3,2,81,'Dodge'),
(3,2,107,'Block'),
(3,2,198,'One-Handed Maces'),
(3,2,199,'Two-Handed Maces'),
(3,2,203,'Unarmed'),
(3,2,204,'Defense'),
(3,2,522,'SPELLDEFENSE (DND)'),
(3,2,635,'Holy Light'),
(3,2,668,'Language Common'),
(3,2,672,'Language Dwarven'),
(3,2,2382,'Generic'),
(3,2,2479,'Honorless Target'),
(3,2,2481,'Find Treasure'),
(3,2,3050,'Detect'),
(3,2,3365,'Opening'),
(3,2,6233,'Closing'),
(3,2,6246,'Closing'),
(3,2,6247,'Opening'),
(3,2,6477,'Opening'),
(3,2,6478,'Opening'),
(3,2,6603,'Attack'),
(3,2,7266,'Duel'),
(3,2,7267,'Grovel'),
(3,2,7355,'Stuck'),
(3,2,8386,'Attacking'),
(3,2,8737,'Mail'),
(3,2,9077,'Leather'),
(3,2,9078,'Cloth'),
(3,2,9116,'Shield'),
(3,2,9125,'Generic'),
(3,2,20154,'Seal of Righteousness'),
(3,2,20594,'Stoneform'),
(3,2,20595,'Gun Specialization'),
(3,2,20596,'Frost Resistance'),
(3,2,21651,'Opening'),
(3,2,21652,'Closing'),
(3,2,22027,'Remove Insignia'),
(3,2,22810,'Opening - No Text'),
(3,2,27762,'Libram'),
(3,3,75,'Auto Shot'),
(3,3,81,'Dodge'),
(3,3,196,'One-Handed Axes'),
(3,3,203,'Unarmed'),
(3,3,204,'Defense'),
(3,3,266,'Guns'),
(3,3,522,'SPELLDEFENSE (DND)'),
(3,3,668,'Language Common'),
(3,3,672,'Language Dwarven'),
(3,3,2382,'Generic'),
(3,3,2479,'Honorless Target'),
(3,3,2481,'Find Treasure'),
(3,3,2973,'Raptor Strike'),
(3,3,3050,'Detect'),
(3,3,3365,'Opening'),
(3,3,6233,'Closing'),
(3,3,6246,'Closing'),
(3,3,6247,'Opening'),
(3,3,6477,'Opening'),
(3,3,6478,'Opening'),
(3,3,6603,'Attack'),
(3,3,7266,'Duel'),
(3,3,7267,'Grovel'),
(3,3,7355,'Stuck'),
(3,3,8386,'Attacking'),
(3,3,9077,'Leather'),
(3,3,9078,'Cloth'),
(3,3,9125,'Generic'),
(3,3,13358,'Defensive State (DND)'),
(3,3,20594,'Stoneform'),
(3,3,20595,'Gun Specialization'),
(3,3,20596,'Frost Resistance'),
(3,3,21651,'Opening'),
(3,3,21652,'Closing'),
(3,3,22027,'Remove Insignia'),
(3,3,22810,'Opening - No Text'),
(3,3,24949,'Defensive State 2 (DND)'),
(3,4,81,'Dodge'),
(3,4,203,'Unarmed'),
(3,4,204,'Defense'),
(3,4,522,'SPELLDEFENSE (DND)'),
(3,4,668,'Language Common'),
(3,4,672,'Language Dwarven'),
(3,4,1180,'Daggers'),
(3,4,1752,'Sinister Strike'),
(3,4,2098,'Eviscerate'),
(3,4,2382,'Generic'),
(3,4,2479,'Honorless Target'),
(3,4,2481,'Find Treasure'),
(3,4,2567,'Thrown'),
(3,4,2764,'Throw'),
(3,4,3050,'Detect'),
(3,4,3365,'Opening'),
(3,4,6233,'Closing'),
(3,4,6246,'Closing'),
(3,4,6247,'Opening'),
(3,4,6477,'Opening'),
(3,4,6478,'Opening'),
(3,4,6603,'Attack'),
(3,4,7266,'Duel'),
(3,4,7267,'Grovel'),
(3,4,7355,'Stuck'),
(3,4,8386,'Attacking'),
(3,4,9077,'Leather'),
(3,4,9078,'Cloth'),
(3,4,9125,'Generic'),
(3,4,16092,'Defensive State (DND)'),
(3,4,20594,'Stoneform'),
(3,4,20595,'Gun Specialization'),
(3,4,20596,'Frost Resistance'),
(3,4,21184,'Rogue Passive (DND)'),
(3,4,21651,'Opening'),
(3,4,21652,'Closing'),
(3,4,22027,'Remove Insignia'),
(3,4,22810,'Opening - No Text'),
(3,5,81,'Dodge'),
(3,5,198,'One-Handed Maces'),
(3,5,203,'Unarmed'),
(3,5,204,'Defense'),
(3,5,522,'SPELLDEFENSE (DND)'),
(3,5,585,'Smite'),
(3,5,668,'Language Common'),
(3,5,672,'Language Dwarven'),
(3,5,2050,'Lesser Heal'),
(3,5,2382,'Generic'),
(3,5,2479,'Honorless Target'),
(3,5,2481,'Find Treasure'),
(3,5,3050,'Detect'),
(3,5,3365,'Opening'),
(3,5,5009,'Wands'),
(3,5,5019,'Shoot'),
(3,5,6233,'Closing'),
(3,5,6246,'Closing'),
(3,5,6247,'Opening'),
(3,5,6477,'Opening'),
(3,5,6478,'Opening'),
(3,5,6603,'Attack'),
(3,5,7266,'Duel'),
(3,5,7267,'Grovel'),
(3,5,7355,'Stuck'),
(3,5,8386,'Attacking'),
(3,5,9078,'Cloth'),
(3,5,9125,'Generic'),
(3,5,20594,'Stoneform'),
(3,5,20595,'Gun Specialization'),
(3,5,20596,'Frost Resistance'),
(3,5,21651,'Opening'),
(3,5,21652,'Closing'),
(3,5,22027,'Remove Insignia'),
(3,5,22810,'Opening - No Text'),
(4,1,78,'Heroic Strike'),
(4,1,81,'Dodge'),
(4,1,107,'Block'),
(4,1,198,'One-Handed Maces'),
(4,1,201,'One-Handed Swords'),
(4,1,203,'Unarmed'),
(4,1,204,'Defense'),
(4,1,522,'SPELLDEFENSE (DND)'),
(4,1,668,'Language Common'),
(4,1,671,'Language Darnassian'),
(4,1,1180,'Daggers'),
(4,1,2382,'Generic'),
(4,1,2457,'Battle Stance'),
(4,1,2479,'Honorless Target'),
(4,1,3050,'Detect'),
(4,1,3365,'Opening'),
(4,1,5301,'Defensive State (DND)'),
(4,1,6233,'Closing'),
(4,1,6246,'Closing'),
(4,1,6247,'Opening'),
(4,1,6477,'Opening'),
(4,1,6478,'Opening'),
(4,1,6603,'Attack'),
(4,1,7266,'Duel'),
(4,1,7267,'Grovel'),
(4,1,7355,'Stuck'),
(4,1,8386,'Attacking'),
(4,1,8737,'Mail'),
(4,1,9077,'Leather'),
(4,1,9078,'Cloth'),
(4,1,9116,'Shield'),
(4,1,9125,'Generic'),
(4,1,20580,'Shadowmeld'),
(4,1,20582,'Quickness'),
(4,1,20583,'Nature Resistance'),
(4,1,20585,'Wisp Spirit'),
(4,1,21009,'Shadowmeld Passive'),
(4,1,21651,'Opening'),
(4,1,21652,'Closing'),
(4,1,22027,'Remove Insignia'),
(4,1,22810,'Opening - No Text'),
(4,3,75,'Auto Shot'),
(4,3,81,'Dodge'),
(4,3,203,'Unarmed'),
(4,3,204,'Defense'),
(4,3,264,'Bows'),
(4,3,522,'SPELLDEFENSE (DND)'),
(4,3,668,'Language Common'),
(4,3,671,'Language Darnassian'),
(4,3,1180,'Daggers'),
(4,3,2382,'Generic'),
(4,3,2479,'Honorless Target'),
(4,3,2973,'Raptor Strike'),
(4,3,3050,'Detect'),
(4,3,3365,'Opening'),
(4,3,6233,'Closing'),
(4,3,6246,'Closing'),
(4,3,6247,'Opening'),
(4,3,6477,'Opening'),
(4,3,6478,'Opening'),
(4,3,6603,'Attack'),
(4,3,7266,'Duel'),
(4,3,7267,'Grovel'),
(4,3,7355,'Stuck'),
(4,3,8386,'Attacking'),
(4,3,9077,'Leather'),
(4,3,9078,'Cloth'),
(4,3,9125,'Generic'),
(4,3,13358,'Defensive State (DND)'),
(4,3,20580,'Shadowmeld'),
(4,3,20582,'Quickness'),
(4,3,20583,'Nature Resistance'),
(4,3,20585,'Wisp Spirit'),
(4,3,21009,'Shadowmeld Passive'),
(4,3,21651,'Opening'),
(4,3,21652,'Closing'),
(4,3,22027,'Remove Insignia'),
(4,3,22810,'Opening - No Text'),
(4,3,24949,'Defensive State 2 (DND)'),
(4,4,81,'Dodge'),
(4,4,203,'Unarmed'),
(4,4,204,'Defense'),
(4,4,522,'SPELLDEFENSE (DND)'),
(4,4,668,'Language Common'),
(4,4,671,'Language Darnassian'),
(4,4,1180,'Daggers'),
(4,4,1752,'Sinister Strike'),
(4,4,2098,'Eviscerate'),
(4,4,2382,'Generic'),
(4,4,2479,'Honorless Target'),
(4,4,2567,'Thrown'),
(4,4,2764,'Throw'),
(4,4,3050,'Detect'),
(4,4,3365,'Opening'),
(4,4,6233,'Closing'),
(4,4,6246,'Closing'),
(4,4,6247,'Opening'),
(4,4,6477,'Opening'),
(4,4,6478,'Opening'),
(4,4,6603,'Attack'),
(4,4,7266,'Duel'),
(4,4,7267,'Grovel'),
(4,4,7355,'Stuck'),
(4,4,8386,'Attacking'),
(4,4,9077,'Leather'),
(4,4,9078,'Cloth'),
(4,4,9125,'Generic'),
(4,4,16092,'Defensive State (DND)'),
(4,4,20580,'Shadowmeld'),
(4,4,20582,'Quickness'),
(4,4,20583,'Nature Resistance'),
(4,4,20585,'Wisp Spirit'),
(4,4,21009,'Shadowmeld Passive'),
(4,4,21184,'Rogue Passive (DND)'),
(4,4,21651,'Opening'),
(4,4,21652,'Closing'),
(4,4,22027,'Remove Insignia'),
(4,4,22810,'Opening - No Text'),
(4,5,81,'Dodge'),
(4,5,198,'One-Handed Maces'),
(4,5,203,'Unarmed'),
(4,5,204,'Defense'),
(4,5,522,'SPELLDEFENSE (DND)'),
(4,5,585,'Smite'),
(4,5,668,'Language Common'),
(4,5,671,'Language Darnassian'),
(4,5,2050,'Lesser Heal'),
(4,5,2382,'Generic'),
(4,5,2479,'Honorless Target'),
(4,5,3050,'Detect'),
(4,5,3365,'Opening'),
(4,5,5009,'Wands'),
(4,5,5019,'Shoot'),
(4,5,6233,'Closing'),
(4,5,6246,'Closing'),
(4,5,6247,'Opening'),
(4,5,6477,'Opening'),
(4,5,6478,'Opening'),
(4,5,6603,'Attack'),
(4,5,7266,'Duel'),
(4,5,7267,'Grovel'),
(4,5,7355,'Stuck'),
(4,5,8386,'Attacking'),
(4,5,9078,'Cloth'),
(4,5,9125,'Generic'),
(4,5,20580,'Shadowmeld'),
(4,5,20582,'Quickness'),
(4,5,20583,'Nature Resistance'),
(4,5,20585,'Wisp Spirit'),
(4,5,21009,'Shadowmeld Passive'),
(4,5,21651,'Opening'),
(4,5,21652,'Closing'),
(4,5,22027,'Remove Insignia'),
(4,5,22810,'Opening - No Text'),
(4,11,81,'Dodge'),
(4,11,203,'Unarmed'),
(4,11,204,'Defense'),
(4,11,227,'Staves'),
(4,11,522,'SPELLDEFENSE (DND)'),
(4,11,668,'Language Common'),
(4,11,671,'Language Darnassian'),
(4,11,1180,'Daggers'),
(4,11,2382,'Generic'),
(4,11,2479,'Honorless Target'),
(4,11,3050,'Detect'),
(4,11,3365,'Opening'),
(4,11,5176,'Wrath'),
(4,11,5185,'Healing Touch'),
(4,11,6233,'Closing'),
(4,11,6246,'Closing'),
(4,11,6247,'Opening'),
(4,11,6477,'Opening'),
(4,11,6478,'Opening'),
(4,11,6603,'Attack'),
(4,11,7266,'Duel'),
(4,11,7267,'Grovel'),
(4,11,7355,'Stuck'),
(4,11,8386,'Attacking'),
(4,11,9077,'Leather'),
(4,11,9078,'Cloth'),
(4,11,9125,'Generic'),
(4,11,20580,'Shadowmeld'),
(4,11,20582,'Quickness'),
(4,11,20583,'Nature Resistance'),
(4,11,20585,'Wisp Spirit'),
(4,11,21009,'Shadowmeld Passive'),
(4,11,21651,'Opening'),
(4,11,21652,'Closing'),
(4,11,22027,'Remove Insignia'),
(4,11,22810,'Opening - No Text'),
(4,11,27764,'Fetish'),
(5,1,78,'Heroic Strike'),
(5,1,81,'Dodge'),
(5,1,107,'Block'),
(5,1,201,'One-Handed Swords'),
(5,1,202,'Two-Handed Swords'),
(5,1,203,'Unarmed'),
(5,1,204,'Defense'),
(5,1,522,'SPELLDEFENSE (DND)'),
(5,1,669,'Language Orcish'),
(5,1,1180,'Daggers'),
(5,1,2382,'Generic'),
(5,1,2457,'Battle Stance'),
(5,1,2479,'Honorless Target'),
(5,1,3050,'Detect'),
(5,1,3365,'Opening'),
(5,1,5227,'Underwater Breathing'),
(5,1,5301,'Defensive State (DND)'),
(5,1,6233,'Closing'),
(5,1,6246,'Closing'),
(5,1,6247,'Opening'),
(5,1,6477,'Opening'),
(5,1,6478,'Opening'),
(5,1,6603,'Attack'),
(5,1,7266,'Duel'),
(5,1,7267,'Grovel'),
(5,1,7355,'Stuck'),
(5,1,7744,'Will of the Forsaken'),
(5,1,8386,'Attacking'),
(5,1,8737,'Mail'),
(5,1,9077,'Leather'),
(5,1,9078,'Cloth'),
(5,1,9116,'Shield'),
(5,1,9125,'Generic'),
(5,1,17737,'Language Gutterspeak'),
(5,1,20577,'Cannibalize'),
(5,1,20579,'Shadow Resistance'),
(5,1,21651,'Opening'),
(5,1,21652,'Closing'),
(5,1,22027,'Remove Insignia'),
(5,1,22810,'Opening - No Text'),
(5,4,81,'Dodge'),
(5,4,203,'Unarmed'),
(5,4,204,'Defense'),
(5,4,522,'SPELLDEFENSE (DND)'),
(5,4,669,'Language Orcish'),
(5,4,1180,'Daggers'),
(5,4,1752,'Sinister Strike'),
(5,4,2098,'Eviscerate'),
(5,4,2382,'Generic'),
(5,4,2479,'Honorless Target'),
(5,4,2567,'Thrown'),
(5,4,2764,'Throw'),
(5,4,3050,'Detect'),
(5,4,3365,'Opening'),
(5,4,5227,'Underwater Breathing'),
(5,4,6233,'Closing'),
(5,4,6246,'Closing'),
(5,4,6247,'Opening'),
(5,4,6477,'Opening'),
(5,4,6478,'Opening'),
(5,4,6603,'Attack'),
(5,4,7266,'Duel'),
(5,4,7267,'Grovel'),
(5,4,7355,'Stuck'),
(5,4,7744,'Will of the Forsaken'),
(5,4,8386,'Attacking'),
(5,4,9077,'Leather'),
(5,4,9078,'Cloth'),
(5,4,9125,'Generic'),
(5,4,16092,'Defensive State (DND)'),
(5,4,17737,'Language Gutterspeak'),
(5,4,20577,'Cannibalize'),
(5,4,20579,'Shadow Resistance'),
(5,4,21184,'Rogue Passive (DND)'),
(5,4,21651,'Opening'),
(5,4,21652,'Closing'),
(5,4,22027,'Remove Insignia'),
(5,4,22810,'Opening - No Text'),
(5,5,81,'Dodge'),
(5,5,198,'One-Handed Maces'),
(5,5,203,'Unarmed'),
(5,5,204,'Defense'),
(5,5,522,'SPELLDEFENSE (DND)'),
(5,5,585,'Smite'),
(5,5,669,'Language Orcish'),
(5,5,2050,'Lesser Heal'),
(5,5,2382,'Generic'),
(5,5,2479,'Honorless Target'),
(5,5,3050,'Detect'),
(5,5,3365,'Opening'),
(5,5,5009,'Wands'),
(5,5,5019,'Shoot'),
(5,5,5227,'Underwater Breathing'),
(5,5,6233,'Closing'),
(5,5,6246,'Closing'),
(5,5,6247,'Opening'),
(5,5,6477,'Opening'),
(5,5,6478,'Opening'),
(5,5,6603,'Attack'),
(5,5,7266,'Duel'),
(5,5,7267,'Grovel'),
(5,5,7355,'Stuck'),
(5,5,7744,'Will of the Forsaken'),
(5,5,8386,'Attacking'),
(5,5,9078,'Cloth'),
(5,5,9125,'Generic'),
(5,5,17737,'Language Gutterspeak'),
(5,5,20577,'Cannibalize'),
(5,5,20579,'Shadow Resistance'),
(5,5,21651,'Opening'),
(5,5,21652,'Closing'),
(5,5,22027,'Remove Insignia'),
(5,5,22810,'Opening - No Text'),
(5,8,81,'Dodge'),
(5,8,133,'Fireball'),
(5,8,168,'Frost Armor'),
(5,8,203,'Unarmed'),
(5,8,204,'Defense'),
(5,8,227,'Staves'),
(5,8,522,'SPELLDEFENSE (DND)'),
(5,8,669,'Language Orcish'),
(5,8,2382,'Generic'),
(5,8,2479,'Honorless Target'),
(5,8,3050,'Detect'),
(5,8,3365,'Opening'),
(5,8,5009,'Wands'),
(5,8,5019,'Shoot'),
(5,8,5227,'Underwater Breathing'),
(5,8,6233,'Closing'),
(5,8,6246,'Closing'),
(5,8,6247,'Opening'),
(5,8,6477,'Opening'),
(5,8,6478,'Opening'),
(5,8,6603,'Attack'),
(5,8,7266,'Duel'),
(5,8,7267,'Grovel'),
(5,8,7355,'Stuck'),
(5,8,7744,'Will of the Forsaken'),
(5,8,8386,'Attacking'),
(5,8,9078,'Cloth'),
(5,8,9125,'Generic'),
(5,8,17737,'Language Gutterspeak'),
(5,8,20577,'Cannibalize'),
(5,8,20579,'Shadow Resistance'),
(5,8,21651,'Opening'),
(5,8,21652,'Closing'),
(5,8,22027,'Remove Insignia'),
(5,8,22810,'Opening - No Text'),
(5,9,81,'Dodge'),
(5,9,203,'Unarmed'),
(5,9,204,'Defense'),
(5,9,522,'SPELLDEFENSE (DND)'),
(5,9,669,'Language Orcish'),
(5,9,686,'Shadow Bolt'),
(5,9,687,'Demon Skin'),
(5,9,1180,'Daggers'),
(5,9,2382,'Generic'),
(5,9,2479,'Honorless Target'),
(5,9,3050,'Detect'),
(5,9,3365,'Opening'),
(5,9,5009,'Wands'),
(5,9,5019,'Shoot'),
(5,9,5227,'Underwater Breathing'),
(5,9,6233,'Closing'),
(5,9,6246,'Closing'),
(5,9,6247,'Opening'),
(5,9,6477,'Opening'),
(5,9,6478,'Opening'),
(5,9,6603,'Attack'),
(5,9,7266,'Duel'),
(5,9,7267,'Grovel'),
(5,9,7355,'Stuck'),
(5,9,7744,'Will of the Forsaken'),
(5,9,8386,'Attacking'),
(5,9,9078,'Cloth'),
(5,9,9125,'Generic'),
(5,9,17737,'Language Gutterspeak'),
(5,9,20577,'Cannibalize'),
(5,9,20579,'Shadow Resistance'),
(5,9,21651,'Opening'),
(5,9,21652,'Closing'),
(5,9,22027,'Remove Insignia'),
(5,9,22810,'Opening - No Text'),
(6,1,78,'Heroic Strike'),
(6,1,81,'Dodge'),
(6,1,107,'Block'),
(6,1,196,'One-Handed Axes'),
(6,1,198,'One-Handed Maces'),
(6,1,199,'Two-Handed Maces'),
(6,1,203,'Unarmed'),
(6,1,204,'Defense'),
(6,1,522,'SPELLDEFENSE (DND)'),
(6,1,669,'Language Orcish'),
(6,1,670,'Language Taurahe'),
(6,1,2382,'Generic'),
(6,1,2457,'Battle Stance'),
(6,1,2479,'Honorless Target'),
(6,1,3050,'Detect'),
(6,1,3365,'Opening'),
(6,1,5301,'Defensive State (DND)'),
(6,1,6233,'Closing'),
(6,1,6246,'Closing'),
(6,1,6247,'Opening'),
(6,1,6477,'Opening'),
(6,1,6478,'Opening'),
(6,1,6603,'Attack'),
(6,1,7266,'Duel'),
(6,1,7267,'Grovel'),
(6,1,7355,'Stuck'),
(6,1,8386,'Attacking'),
(6,1,8737,'Mail'),
(6,1,9077,'Leather'),
(6,1,9078,'Cloth'),
(6,1,9116,'Shield'),
(6,1,9125,'Generic'),
(6,1,20549,'War Stomp'),
(6,1,20550,'Endurance'),
(6,1,20551,'Nature Resistance'),
(6,1,20552,'Cultivation'),
(6,1,21651,'Opening'),
(6,1,21652,'Closing'),
(6,1,22027,'Remove Insignia'),
(6,1,22810,'Opening - No Text'),
(6,3,75,'Auto Shot'),
(6,3,81,'Dodge'),
(6,3,196,'One-Handed Axes'),
(6,3,203,'Unarmed'),
(6,3,204,'Defense'),
(6,3,266,'Guns'),
(6,3,522,'SPELLDEFENSE (DND)'),
(6,3,669,'Language Orcish'),
(6,3,670,'Language Taurahe'),
(6,3,2382,'Generic'),
(6,3,2479,'Honorless Target'),
(6,3,2973,'Raptor Strike'),
(6,3,3050,'Detect'),
(6,3,3365,'Opening'),
(6,3,6233,'Closing'),
(6,3,6246,'Closing'),
(6,3,6247,'Opening'),
(6,3,6477,'Opening'),
(6,3,6478,'Opening'),
(6,3,6603,'Attack'),
(6,3,7266,'Duel'),
(6,3,7267,'Grovel'),
(6,3,7355,'Stuck'),
(6,3,8386,'Attacking'),
(6,3,9077,'Leather'),
(6,3,9078,'Cloth'),
(6,3,9125,'Generic'),
(6,3,13358,'Defensive State (DND)'),
(6,3,20549,'War Stomp'),
(6,3,20550,'Endurance'),
(6,3,20551,'Nature Resistance'),
(6,3,20552,'Cultivation'),
(6,3,21651,'Opening'),
(6,3,21652,'Closing'),
(6,3,22027,'Remove Insignia'),
(6,3,22810,'Opening - No Text'),
(6,3,24949,'Defensive State 2 (DND)'),
(6,7,81,'Dodge'),
(6,7,107,'Block'),
(6,7,198,'One-Handed Maces'),
(6,7,203,'Unarmed'),
(6,7,204,'Defense'),
(6,7,227,'Staves'),
(6,7,331,'Healing Wave'),
(6,7,403,'Lightning Bolt'),
(6,7,522,'SPELLDEFENSE (DND)'),
(6,7,669,'Language Orcish'),
(6,7,670,'Language Taurahe'),
(6,7,2382,'Generic'),
(6,7,2479,'Honorless Target'),
(6,7,3050,'Detect'),
(6,7,3365,'Opening'),
(6,7,6233,'Closing'),
(6,7,6246,'Closing'),
(6,7,6247,'Opening'),
(6,7,6477,'Opening'),
(6,7,6478,'Opening'),
(6,7,6603,'Attack'),
(6,7,7266,'Duel'),
(6,7,7267,'Grovel'),
(6,7,7355,'Stuck'),
(6,7,8386,'Attacking'),
(6,7,9077,'Leather'),
(6,7,9078,'Cloth'),
(6,7,9116,'Shield'),
(6,7,9125,'Generic'),
(6,7,20549,'War Stomp'),
(6,7,20550,'Endurance'),
(6,7,20551,'Nature Resistance'),
(6,7,20552,'Cultivation'),
(6,7,21651,'Opening'),
(6,7,21652,'Closing'),
(6,7,22027,'Remove Insignia'),
(6,7,22810,'Opening - No Text'),
(6,7,27763,'Totem'),
(6,11,81,'Dodge'),
(6,11,198,'One-Handed Maces'),
(6,11,203,'Unarmed'),
(6,11,204,'Defense'),
(6,11,227,'Staves'),
(6,11,522,'SPELLDEFENSE (DND)'),
(6,11,669,'Language Orcish'),
(6,11,670,'Language Taurahe'),
(6,11,2382,'Generic'),
(6,11,2479,'Honorless Target'),
(6,11,3050,'Detect'),
(6,11,3365,'Opening'),
(6,11,5176,'Wrath'),
(6,11,5185,'Healing Touch'),
(6,11,6233,'Closing'),
(6,11,6246,'Closing'),
(6,11,6247,'Opening'),
(6,11,6477,'Opening'),
(6,11,6478,'Opening'),
(6,11,6603,'Attack'),
(6,11,7266,'Duel'),
(6,11,7267,'Grovel'),
(6,11,7355,'Stuck'),
(6,11,8386,'Attacking'),
(6,11,9077,'Leather'),
(6,11,9078,'Cloth'),
(6,11,9125,'Generic'),
(6,11,20549,'War Stomp'),
(6,11,20550,'Endurance'),
(6,11,20551,'Nature Resistance'),
(6,11,20552,'Cultivation'),
(6,11,21651,'Opening'),
(6,11,21652,'Closing'),
(6,11,22027,'Remove Insignia'),
(6,11,22810,'Opening - No Text'),
(6,11,27764,'Fetish'),
(7,1,78,'Heroic Strike'),
(7,1,81,'Dodge'),
(7,1,107,'Block'),
(7,1,198,'One-Handed Maces'),
(7,1,201,'One-Handed Swords'),
(7,1,203,'Unarmed'),
(7,1,204,'Defense'),
(7,1,522,'SPELLDEFENSE (DND)'),
(7,1,668,'Language Common'),
(7,1,1180,'Daggers'),
(7,1,2382,'Generic'),
(7,1,2457,'Battle Stance'),
(7,1,2479,'Honorless Target'),
(7,1,3050,'Detect'),
(7,1,3365,'Opening'),
(7,1,5301,'Defensive State (DND)'),
(7,1,6233,'Closing'),
(7,1,6246,'Closing'),
(7,1,6247,'Opening'),
(7,1,6477,'Opening'),
(7,1,6478,'Opening'),
(7,1,6603,'Attack'),
(7,1,7266,'Duel'),
(7,1,7267,'Grovel'),
(7,1,7340,'Language Gnomish'),
(7,1,7355,'Stuck'),
(7,1,8386,'Attacking'),
(7,1,8737,'Mail'),
(7,1,9077,'Leather'),
(7,1,9078,'Cloth'),
(7,1,9116,'Shield'),
(7,1,9125,'Generic'),
(7,1,20589,'Escape Artist'),
(7,1,20591,'Expansive Mind'),
(7,1,20592,'Arcane Resistance'),
(7,1,20593,'Engineering Specialization'),
(7,1,21651,'Opening'),
(7,1,21652,'Closing'),
(7,1,22027,'Remove Insignia'),
(7,1,22810,'Opening - No Text'),
(7,4,81,'Dodge'),
(7,4,203,'Unarmed'),
(7,4,204,'Defense'),
(7,4,522,'SPELLDEFENSE (DND)'),
(7,4,668,'Language Common'),
(7,4,1180,'Daggers'),
(7,4,1752,'Sinister Strike'),
(7,4,2098,'Eviscerate'),
(7,4,2382,'Generic'),
(7,4,2479,'Honorless Target'),
(7,4,2567,'Thrown'),
(7,4,2764,'Throw'),
(7,4,3050,'Detect'),
(7,4,3365,'Opening'),
(7,4,6233,'Closing'),
(7,4,6246,'Closing'),
(7,4,6247,'Opening'),
(7,4,6477,'Opening'),
(7,4,6478,'Opening'),
(7,4,6603,'Attack'),
(7,4,7266,'Duel'),
(7,4,7267,'Grovel'),
(7,4,7340,'Language Gnomish'),
(7,4,7355,'Stuck'),
(7,4,8386,'Attacking'),
(7,4,9077,'Leather'),
(7,4,9078,'Cloth'),
(7,4,9125,'Generic'),
(7,4,16092,'Defensive State (DND)'),
(7,4,20589,'Escape Artist'),
(7,4,20591,'Expansive Mind'),
(7,4,20592,'Arcane Resistance'),
(7,4,20593,'Engineering Specialization'),
(7,4,21184,'Rogue Passive (DND)'),
(7,4,21651,'Opening'),
(7,4,21652,'Closing'),
(7,4,22027,'Remove Insignia'),
(7,4,22810,'Opening - No Text'),
(7,8,81,'Dodge'),
(7,8,133,'Fireball'),
(7,8,168,'Frost Armor'),
(7,8,203,'Unarmed'),
(7,8,204,'Defense'),
(7,8,227,'Staves'),
(7,8,522,'SPELLDEFENSE (DND)'),
(7,8,668,'Language Common'),
(7,8,2382,'Generic'),
(7,8,2479,'Honorless Target'),
(7,8,3050,'Detect'),
(7,8,3365,'Opening'),
(7,8,5009,'Wands'),
(7,8,5019,'Shoot'),
(7,8,6233,'Closing'),
(7,8,6246,'Closing'),
(7,8,6247,'Opening'),
(7,8,6477,'Opening'),
(7,8,6478,'Opening'),
(7,8,6603,'Attack'),
(7,8,7266,'Duel'),
(7,8,7267,'Grovel'),
(7,8,7340,'Language Gnomish'),
(7,8,7355,'Stuck'),
(7,8,8386,'Attacking'),
(7,8,9078,'Cloth'),
(7,8,9125,'Generic'),
(7,8,20589,'Escape Artist'),
(7,8,20591,'Expansive Mind'),
(7,8,20592,'Arcane Resistance'),
(7,8,20593,'Engineering Specialization'),
(7,8,21651,'Opening'),
(7,8,21652,'Closing'),
(7,8,22027,'Remove Insignia'),
(7,8,22810,'Opening - No Text'),
(7,9,81,'Dodge'),
(7,9,203,'Unarmed'),
(7,9,204,'Defense'),
(7,9,522,'SPELLDEFENSE (DND)'),
(7,9,668,'Language Common'),
(7,9,686,'Shadow Bolt'),
(7,9,687,'Demon Skin'),
(7,9,1180,'Daggers'),
(7,9,2382,'Generic'),
(7,9,2479,'Honorless Target'),
(7,9,3050,'Detect'),
(7,9,3365,'Opening'),
(7,9,5009,'Wands'),
(7,9,5019,'Shoot'),
(7,9,6233,'Closing'),
(7,9,6246,'Closing'),
(7,9,6247,'Opening'),
(7,9,6477,'Opening'),
(7,9,6478,'Opening'),
(7,9,6603,'Attack'),
(7,9,7266,'Duel'),
(7,9,7267,'Grovel'),
(7,9,7340,'Language Gnomish'),
(7,9,7355,'Stuck'),
(7,9,8386,'Attacking'),
(7,9,9078,'Cloth'),
(7,9,9125,'Generic'),
(7,9,20589,'Escape Artist'),
(7,9,20591,'Expansive Mind'),
(7,9,20592,'Arcane Resistance'),
(7,9,20593,'Engineering Specialization'),
(7,9,21651,'Opening'),
(7,9,21652,'Closing'),
(7,9,22027,'Remove Insignia'),
(7,9,22810,'Opening - No Text'),
(8,1,78,'Heroic Strike'),
(8,1,81,'Dodge'),
(8,1,107,'Block'),
(8,1,196,'One-Handed Axes'),
(8,1,203,'Unarmed'),
(8,1,204,'Defense'),
(8,1,522,'SPELLDEFENSE (DND)'),
(8,1,669,'Language Orcish'),
(8,1,1180,'Daggers'),
(8,1,2382,'Generic'),
(8,1,2457,'Battle Stance'),
(8,1,2479,'Honorless Target'),
(8,1,2567,'Thrown'),
(8,1,2764,'Throw'),
(8,1,3050,'Detect'),
(8,1,3365,'Opening'),
(8,1,5301,'Defensive State (DND)'),
(8,1,6233,'Closing'),
(8,1,6246,'Closing'),
(8,1,6247,'Opening'),
(8,1,6477,'Opening'),
(8,1,6478,'Opening'),
(8,1,6603,'Attack'),
(8,1,7266,'Duel'),
(8,1,7267,'Grovel'),
(8,1,7341,'Language Troll'),
(8,1,7355,'Stuck'),
(8,1,8386,'Attacking'),
(8,1,8737,'Mail'),
(8,1,9077,'Leather'),
(8,1,9078,'Cloth'),
(8,1,9116,'Shield'),
(8,1,9125,'Generic'),
(8,1,20555,'Regeneration'),
(8,1,20557,'Beast Slaying'),
(8,1,20558,'Throwing Specialization'),
(8,1,21651,'Opening'),
(8,1,21652,'Closing'),
(8,1,22027,'Remove Insignia'),
(8,1,22810,'Opening - No Text'),
(8,1,26290,'Bow Specialization'),
(8,1,26296,'Berserking'),
(8,3,75,'Auto Shot'),
(8,3,81,'Dodge'),
(8,3,196,'One-Handed Axes'),
(8,3,203,'Unarmed'),
(8,3,204,'Defense'),
(8,3,264,'Bows'),
(8,3,522,'SPELLDEFENSE (DND)'),
(8,3,669,'Language Orcish'),
(8,3,2382,'Generic'),
(8,3,2479,'Honorless Target'),
(8,3,2973,'Raptor Strike'),
(8,3,3050,'Detect'),
(8,3,3365,'Opening'),
(8,3,6233,'Closing'),
(8,3,6246,'Closing'),
(8,3,6247,'Opening'),
(8,3,6477,'Opening'),
(8,3,6478,'Opening'),
(8,3,6603,'Attack'),
(8,3,7266,'Duel'),
(8,3,7267,'Grovel'),
(8,3,7341,'Language Troll'),
(8,3,7355,'Stuck'),
(8,3,8386,'Attacking'),
(8,3,9077,'Leather'),
(8,3,9078,'Cloth'),
(8,3,9125,'Generic'),
(8,3,13358,'Defensive State (DND)'),
(8,3,20554,'Berserking'),
(8,3,20555,'Regeneration'),
(8,3,20557,'Beast Slaying'),
(8,3,20558,'Throwing Specialization'),
(8,3,21651,'Opening'),
(8,3,21652,'Closing'),
(8,3,22027,'Remove Insignia'),
(8,3,22810,'Opening - No Text'),
(8,3,24949,'Defensive State 2 (DND)'),
(8,3,26290,'Bow Specialization'),
(8,4,81,'Dodge'),
(8,4,203,'Unarmed'),
(8,4,204,'Defense'),
(8,4,522,'SPELLDEFENSE (DND)'),
(8,4,669,'Language Orcish'),
(8,4,1180,'Daggers'),
(8,4,1752,'Sinister Strike'),
(8,4,2098,'Eviscerate'),
(8,4,2382,'Generic'),
(8,4,2479,'Honorless Target'),
(8,4,2567,'Thrown'),
(8,4,2764,'Throw'),
(8,4,3050,'Detect'),
(8,4,3365,'Opening'),
(8,4,6233,'Closing'),
(8,4,6246,'Closing'),
(8,4,6247,'Opening'),
(8,4,6477,'Opening'),
(8,4,6478,'Opening'),
(8,4,6603,'Attack'),
(8,4,7266,'Duel'),
(8,4,7267,'Grovel'),
(8,4,7341,'Language Troll'),
(8,4,7355,'Stuck'),
(8,4,8386,'Attacking'),
(8,4,9077,'Leather'),
(8,4,9078,'Cloth'),
(8,4,9125,'Generic'),
(8,4,16092,'Defensive State (DND)'),
(8,4,20555,'Regeneration'),
(8,4,20557,'Beast Slaying'),
(8,4,20558,'Throwing Specialization'),
(8,4,21184,'Rogue Passive (DND)'),
(8,4,21651,'Opening'),
(8,4,21652,'Closing'),
(8,4,22027,'Remove Insignia'),
(8,4,22810,'Opening - No Text'),
(8,4,26290,'Bow Specialization'),
(8,4,26297,'Berserking'),
(8,5,81,'Dodge'),
(8,5,198,'One-Handed Maces'),
(8,5,203,'Unarmed'),
(8,5,204,'Defense'),
(8,5,522,'SPELLDEFENSE (DND)'),
(8,5,585,'Smite'),
(8,5,669,'Language Orcish'),
(8,5,2050,'Lesser Heal'),
(8,5,2382,'Generic'),
(8,5,2479,'Honorless Target'),
(8,5,3050,'Detect'),
(8,5,3365,'Opening'),
(8,5,5009,'Wands'),
(8,5,5019,'Shoot'),
(8,5,6233,'Closing'),
(8,5,6246,'Closing'),
(8,5,6247,'Opening'),
(8,5,6477,'Opening'),
(8,5,6478,'Opening'),
(8,5,6603,'Attack'),
(8,5,7266,'Duel'),
(8,5,7267,'Grovel'),
(8,5,7341,'Language Troll'),
(8,5,7355,'Stuck'),
(8,5,8386,'Attacking'),
(8,5,9078,'Cloth'),
(8,5,9125,'Generic'),
(8,5,20554,'Berserking'),
(8,5,20555,'Regeneration'),
(8,5,20557,'Beast Slaying'),
(8,5,20558,'Throwing Specialization'),
(8,5,21651,'Opening'),
(8,5,21652,'Closing'),
(8,5,22027,'Remove Insignia'),
(8,5,22810,'Opening - No Text'),
(8,5,26290,'Bow Specialization'),
(8,7,81,'Dodge'),
(8,7,107,'Block'),
(8,7,198,'One-Handed Maces'),
(8,7,203,'Unarmed'),
(8,7,204,'Defense'),
(8,7,227,'Staves'),
(8,7,331,'Healing Wave'),
(8,7,403,'Lightning Bolt'),
(8,7,522,'SPELLDEFENSE (DND)'),
(8,7,669,'Language Orcish'),
(8,7,2382,'Generic'),
(8,7,2479,'Honorless Target'),
(8,7,3050,'Detect'),
(8,7,3365,'Opening'),
(8,7,6233,'Closing'),
(8,7,6246,'Closing'),
(8,7,6247,'Opening'),
(8,7,6477,'Opening'),
(8,7,6478,'Opening'),
(8,7,6603,'Attack'),
(8,7,7266,'Duel'),
(8,7,7267,'Grovel'),
(8,7,7341,'Language Troll'),
(8,7,7355,'Stuck'),
(8,7,8386,'Attacking'),
(8,7,9077,'Leather'),
(8,7,9078,'Cloth'),
(8,7,9116,'Shield'),
(8,7,9125,'Generic'),
(8,7,20554,'Berserking'),
(8,7,20555,'Regeneration'),
(8,7,20557,'Beast Slaying'),
(8,7,20558,'Throwing Specialization'),
(8,7,21651,'Opening'),
(8,7,21652,'Closing'),
(8,7,22027,'Remove Insignia'),
(8,7,22810,'Opening - No Text'),
(8,7,26290,'Bow Specialization'),
(8,7,27763,'Totem'),
(8,8,81,'Dodge'),
(8,8,133,'Fireball'),
(8,8,168,'Frost Armor'),
(8,8,203,'Unarmed'),
(8,8,204,'Defense'),
(8,8,227,'Staves'),
(8,8,522,'SPELLDEFENSE (DND)'),
(8,8,669,'Language Orcish'),
(8,8,2382,'Generic'),
(8,8,2479,'Honorless Target'),
(8,8,3050,'Detect'),
(8,8,3365,'Opening'),
(8,8,5009,'Wands'),
(8,8,5019,'Shoot'),
(8,8,6233,'Closing'),
(8,8,6246,'Closing'),
(8,8,6247,'Opening'),
(8,8,6477,'Opening'),
(8,8,6478,'Opening'),
(8,8,6603,'Attack'),
(8,8,7266,'Duel'),
(8,8,7267,'Grovel'),
(8,8,7341,'Language Troll'),
(8,8,7355,'Stuck'),
(8,8,8386,'Attacking'),
(8,8,9078,'Cloth'),
(8,8,9125,'Generic'),
(8,8,20554,'Berserking'),
(8,8,20555,'Regeneration'),
(8,8,20557,'Beast Slaying'),
(8,8,20558,'Throwing Specialization'),
(8,8,21651,'Opening'),
(8,8,21652,'Closing'),
(8,8,22027,'Remove Insignia'),
(8,8,22810,'Opening - No Text'),
(8,8,26290,'Bow Specialization');
/*!40000 ALTER TABLE `playercreateinfo_spell` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `points_of_interest`
--

DROP TABLE IF EXISTS `points_of_interest`;
CREATE TABLE `points_of_interest` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `x` float NOT NULL DEFAULT '0',
  `y` float NOT NULL DEFAULT '0',
  `icon` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `flags` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `data` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `icon_name` text NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `points_of_interest`
--

LOCK TABLES `points_of_interest` WRITE;
/*!40000 ALTER TABLE `points_of_interest` DISABLE KEYS */;
/*!40000 ALTER TABLE `points_of_interest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_creature`
--

DROP TABLE IF EXISTS `pool_creature`;
CREATE TABLE `pool_creature` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `pool_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `pool_idx` (`pool_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_creature`
--

LOCK TABLES `pool_creature` WRITE;
/*!40000 ALTER TABLE `pool_creature` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_creature` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_creature_template`
--

DROP TABLE IF EXISTS `pool_creature_template`;
CREATE TABLE `pool_creature_template` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `pool_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `pool_idx` (`pool_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_creature_template`
--

LOCK TABLES `pool_creature_template` WRITE;
/*!40000 ALTER TABLE `pool_creature_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_creature_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_gameobject`
--

DROP TABLE IF EXISTS `pool_gameobject`;
CREATE TABLE `pool_gameobject` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `pool_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `pool_idx` (`pool_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_gameobject`
--

LOCK TABLES `pool_gameobject` WRITE;
/*!40000 ALTER TABLE `pool_gameobject` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_gameobject` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_gameobject_template`
--

DROP TABLE IF EXISTS `pool_gameobject_template`;
CREATE TABLE `pool_gameobject_template` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `pool_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `pool_idx` (`pool_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_gameobject_template`
--

LOCK TABLES `pool_gameobject_template` WRITE;
/*!40000 ALTER TABLE `pool_gameobject_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_gameobject_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_pool`
--

DROP TABLE IF EXISTS `pool_pool`;
CREATE TABLE `pool_pool` (
  `pool_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `mother_pool` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `chance` float NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`pool_id`),
  KEY `pool_idx` (`mother_pool`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_pool`
--

LOCK TABLES `pool_pool` WRITE;
/*!40000 ALTER TABLE `pool_pool` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_pool` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pool_template`
--

DROP TABLE IF EXISTS `pool_template`;
CREATE TABLE `pool_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Pool entry',
  `max_limit` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Max number of objects (0) is no limit',
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `pool_template`
--

LOCK TABLES `pool_template` WRITE;
/*!40000 ALTER TABLE `pool_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `pool_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `quest_template`
--

DROP TABLE IF EXISTS `quest_template`;
CREATE TABLE `quest_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Method` tinyint(3) unsigned NOT NULL DEFAULT '2',
  `ZoneOrSort` smallint(6) NOT NULL DEFAULT '0',
  `MinLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MaxLevel` tinyint(3) unsigned NOT NULL DEFAULT '255',
  `QuestLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Type` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredClasses` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredRaces` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredSkill` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredSkillValue` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredCondition` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `RepObjectiveFaction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RepObjectiveValue` mediumint(9) NOT NULL DEFAULT '0',
  `RequiredMinRepFaction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredMinRepValue` mediumint(9) NOT NULL DEFAULT '0',
  `RequiredMaxRepFaction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RequiredMaxRepValue` mediumint(9) NOT NULL DEFAULT '0',
  `SuggestedPlayers` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `LimitTime` int(10) unsigned NOT NULL DEFAULT '0',
  `QuestFlags` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SpecialFlags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `PrevQuestId` mediumint(9) NOT NULL DEFAULT '0',
  `NextQuestId` mediumint(9) NOT NULL DEFAULT '0',
  `ExclusiveGroup` mediumint(9) NOT NULL DEFAULT '0',
  `BreadcrumbForQuestId` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `NextQuestInChain` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SrcItemId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SrcItemCount` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SrcSpell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Title` text,
  `Details` text,
  `Objectives` text,
  `OfferRewardText` text,
  `RequestItemsText` text,
  `EndText` text,
  `ObjectiveText1` text,
  `ObjectiveText2` text,
  `ObjectiveText3` text,
  `ObjectiveText4` text,
  `ReqItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqSourceId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSourceId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSourceId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSourceId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSourceCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqSourceCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqSourceCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqSourceCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqCreatureOrGOId1` mediumint(9) NOT NULL DEFAULT '0',
  `ReqCreatureOrGOId2` mediumint(9) NOT NULL DEFAULT '0',
  `ReqCreatureOrGOId3` mediumint(9) NOT NULL DEFAULT '0',
  `ReqCreatureOrGOId4` mediumint(9) NOT NULL DEFAULT '0',
  `ReqCreatureOrGOCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqCreatureOrGOCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqCreatureOrGOCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqCreatureOrGOCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqSpellCast1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSpellCast2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSpellCast3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqSpellCast4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount6` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewRepFaction1` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'faction id from Faction.dbc in this case',
  `RewRepFaction2` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'faction id from Faction.dbc in this case',
  `RewRepFaction3` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'faction id from Faction.dbc in this case',
  `RewRepFaction4` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'faction id from Faction.dbc in this case',
  `RewRepFaction5` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'faction id from Faction.dbc in this case',
  `RewRepValue1` mediumint(9) NOT NULL DEFAULT '0',
  `RewRepValue2` mediumint(9) NOT NULL DEFAULT '0',
  `RewRepValue3` mediumint(9) NOT NULL DEFAULT '0',
  `RewRepValue4` mediumint(9) NOT NULL DEFAULT '0',
  `RewRepValue5` mediumint(9) NOT NULL DEFAULT '0',
  `ReputationSpilloverMask` tinyint unsigned NOT NULL DEFAULT '0',
  `RewOrReqMoney` int(11) NOT NULL DEFAULT '0',
  `RewMoneyMaxLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `RewSpell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewSpellCast` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewMailTemplateId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewMailDelaySecs` int(11) unsigned NOT NULL DEFAULT '0',
  `PointMapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `PointX` float NOT NULL DEFAULT '0',
  `PointY` float NOT NULL DEFAULT '0',
  `PointOpt` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `DetailsEmote1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DetailsEmote2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DetailsEmote3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DetailsEmote4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DetailsEmoteDelay1` int(11) unsigned NOT NULL DEFAULT '0',
  `DetailsEmoteDelay2` int(11) unsigned NOT NULL DEFAULT '0',
  `DetailsEmoteDelay3` int(11) unsigned NOT NULL DEFAULT '0',
  `DetailsEmoteDelay4` int(11) unsigned NOT NULL DEFAULT '0',
  `IncompleteEmote` smallint(5) unsigned NOT NULL DEFAULT '0',
  `CompleteEmote` smallint(5) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmote1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmote2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmote3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmote4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmoteDelay1` int(11) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmoteDelay2` int(11) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmoteDelay3` int(11) unsigned NOT NULL DEFAULT '0',
  `OfferRewardEmoteDelay4` int(11) unsigned NOT NULL DEFAULT '0',
  `StartScript` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `CompleteScript` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Quest System';

--
-- Dumping data for table `quest_template`
--

LOCK TABLES `quest_template` WRITE;
/*!40000 ALTER TABLE `quest_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `quest_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `questgiver_greeting`
--

DROP TABLE IF EXISTS `questgiver_greeting`;
CREATE TABLE `questgiver_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Questgiver',
   `Type` INT(11) UNSIGNED NOT NULL COMMENT 'Type of entry',
   `Text` LONGTEXT COMMENT 'Text of the greeting',
   `EmoteId` INT(11) UNSIGNED NOT NULL COMMENT 'Emote ID of greeting',
   `EmoteDelay` INT(11) UNSIGNED NOT NULL COMMENT 'Emote delay of the greeting',
   PRIMARY KEY(`Entry`,`Type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Quest and Gossip system';

--
-- Table structure for table `reference_loot_template`
--

DROP TABLE IF EXISTS `reference_loot_template`;
CREATE TABLE `reference_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `reference_loot_template`
--

LOCK TABLES `reference_loot_template` WRITE;
/*!40000 ALTER TABLE `reference_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `reference_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

CREATE TABLE `reference_loot_template_names` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`)
)COMMENT='Reference Loot Template Names';

--
-- Table structure for table `reputation_reward_rate`
--

DROP TABLE IF EXISTS `reputation_reward_rate`;
CREATE TABLE `reputation_reward_rate` (
  `faction` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest_rate` float NOT NULL DEFAULT '1',
  `creature_rate` float NOT NULL DEFAULT '1',
  `spell_rate` float NOT NULL DEFAULT '1',
  PRIMARY KEY (`faction`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `reputation_reward_rate`
--

LOCK TABLES `reputation_reward_rate` WRITE;
/*!40000 ALTER TABLE `reputation_reward_rate` DISABLE KEYS */;
/*!40000 ALTER TABLE `reputation_reward_rate` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `reputation_spillover_template`
--

DROP TABLE IF EXISTS `reputation_spillover_template`;
CREATE TABLE `reputation_spillover_template` (
  `faction` smallint(6) unsigned NOT NULL DEFAULT '0' COMMENT 'faction entry',
  `faction1` smallint(6) unsigned NOT NULL DEFAULT '0' COMMENT 'faction to give spillover for',
  `rate_1` float NOT NULL DEFAULT '0' COMMENT 'the given rep points * rate',
  `rank_1` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'max rank, above this will not give any spillover',
  `faction2` smallint(6) unsigned NOT NULL DEFAULT '0',
  `rate_2` float NOT NULL DEFAULT '0',
  `rank_2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `faction3` smallint(6) unsigned NOT NULL DEFAULT '0',
  `rate_3` float NOT NULL DEFAULT '0',
  `rank_3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `faction4` smallint(6) unsigned NOT NULL DEFAULT '0',
  `rate_4` float NOT NULL DEFAULT '0',
  `rank_4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`faction`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Reputation spillover reputation gain';

--
-- Dumping data for table `reputation_spillover_template`
--

LOCK TABLES `reputation_spillover_template` WRITE;
/*!40000 ALTER TABLE `reputation_spillover_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `reputation_spillover_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `reserved_name`
--

DROP TABLE IF EXISTS `reserved_name`;
CREATE TABLE `reserved_name` (
  `name` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player Reserved Names';

--
-- Dumping data for table `reserved_name`
--

LOCK TABLES `reserved_name` WRITE;
/*!40000 ALTER TABLE `reserved_name` DISABLE KEYS */;
/*!40000 ALTER TABLE `reserved_name` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `script_texts`
--

DROP TABLE IF EXISTS `script_texts`;
CREATE TABLE `script_texts` (
  `entry` mediumint(8) NOT NULL,
  `content_default` text NOT NULL,
  `content_loc1` text,
  `content_loc2` text,
  `content_loc3` text,
  `content_loc4` text,
  `content_loc5` text,
  `content_loc6` text,
  `content_loc7` text,
  `content_loc8` text,
  `sound` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `language` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `emote` smallint(5) unsigned NOT NULL DEFAULT '0',
  `broadcast_text_id` INT(11) NOT NULL DEFAULT '0',
  `comment` text,
  PRIMARY KEY  (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Script Texts';

--
-- Dumping data for table `script_texts`
--

LOCK TABLES `script_texts` WRITE;
/*!40000 ALTER TABLE `script_texts` DISABLE KEYS */;
/*!40000 ALTER TABLE `script_texts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `script_waypoint`
--

DROP TABLE IF EXISTS `script_waypoint`;
CREATE TABLE script_waypoint (
  `Entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'creature_template entry',
  `PathId` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `Point` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `PositionX` float NOT NULL DEFAULT '0',
  `PositionY` float NOT NULL DEFAULT '0',
  `PositionZ` float NOT NULL DEFAULT '0',
  `Orientation` float NOT NULL DEFAULT '0',
  `WaitTime` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'waittime in millisecs',
  `ScriptId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Comment` text,
  PRIMARY KEY (Entry, PathId, Point)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Script Creature waypoints';

--
-- Dumping data for table `script_waypoint`
--

LOCK TABLES `script_waypoint` WRITE;
/*!40000 ALTER TABLE `script_waypoint` DISABLE KEYS */;
/*!40000 ALTER TABLE `script_waypoint` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `scripted_areatrigger`
--

DROP TABLE IF EXISTS `scripted_areatrigger`;
CREATE TABLE `scripted_areatrigger` (
  `entry` mediumint(8) NOT NULL,
  `ScriptName` char(64) NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `scripted_areatrigger`
--

LOCK TABLES `scripted_areatrigger` WRITE;
/*!40000 ALTER TABLE `scripted_areatrigger` DISABLE KEYS */;
/*!40000 ALTER TABLE `scripted_areatrigger` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `scripted_event_id`
--

DROP TABLE IF EXISTS `scripted_event_id`;
CREATE TABLE `scripted_event_id` (
  `id` mediumint(8) NOT NULL,
  `ScriptName` char(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Script library scripted events';

--
-- Dumping data for table `scripted_event_id`
--

LOCK TABLES `scripted_event_id` WRITE;
/*!40000 ALTER TABLE `scripted_event_id` DISABLE KEYS */;
/*!40000 ALTER TABLE `scripted_event_id` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_fishing_base_level`
--

DROP TABLE IF EXISTS `skill_fishing_base_level`;
CREATE TABLE `skill_fishing_base_level` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Area identifier',
  `skill` smallint(6) NOT NULL DEFAULT '0' COMMENT 'Base skill level requirement',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Fishing system';

--
-- Dumping data for table `skill_fishing_base_level`
--

LOCK TABLES `skill_fishing_base_level` WRITE;
/*!40000 ALTER TABLE `skill_fishing_base_level` DISABLE KEYS */;
/*!40000 ALTER TABLE `skill_fishing_base_level` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skinning_loot_template`
--

DROP TABLE IF EXISTS `skinning_loot_template`;
CREATE TABLE `skinning_loot_template` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ChanceOrQuestChance` float NOT NULL DEFAULT '100',
  `groupid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mincountOrRef` mediumint(9) NOT NULL DEFAULT '1',
  `maxcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `comments` VARCHAR(300) DEFAULT '',
  PRIMARY KEY (`entry`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Loot System';

--
-- Dumping data for table `skinning_loot_template`
--

LOCK TABLES `skinning_loot_template` WRITE;
/*!40000 ALTER TABLE `skinning_loot_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `skinning_loot_template` ENABLE KEYS */;
UNLOCK TABLES;

-- ----------------------------
-- Table structure for spawn_group
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group`;
CREATE TABLE `spawn_group`  (
  `Id` int(11) NOT NULL COMMENT 'Spawn Group ID',
  `Name` varchar(200) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL COMMENT 'Description of usage',
  `Type` int(11) NOT NULL COMMENT 'Creature or GO spawn group',
  `MaxCount` int(11) NOT NULL DEFAULT 0 COMMENT 'Maximum total count of all spawns in a group',
  `WorldState` int(11) NOT NULL DEFAULT 0 COMMENT 'Worldstate which enables spawning of given group',
  `WorldStateExpression` int(11) NOT NULL DEFAULT 0 COMMENT 'Worldstate expression Id',
  `Flags` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Flags for various behaviour',
  `StringId` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `RespawnOverrideMin` INT UNSIGNED COMMENT 'Respawn time override' DEFAULT NULL,
  `RespawnOverrideMax` INT UNSIGNED COMMENT 'Respawn time override' DEFAULT NULL,
  PRIMARY KEY (`Id`)
);

-- ----------------------------
-- Table structure for spawn_group_spawn
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group_spawn`;
CREATE TABLE `spawn_group_spawn`  (
  `Id` int(11) NOT NULL COMMENT 'Spawn Group ID',
  `Guid` int(11) NOT NULL COMMENT 'Guid of creature or GO',
  `SlotId` tinyint(4) NOT NULL DEFAULT -1 COMMENT '0 is the leader, -1 not part of the formation',
  `Chance` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Chance for a spawn to occur',
  PRIMARY KEY (`Id`, `Guid`)
);

-- ----------------------------
-- Table structure for spawn_group_entry
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group_entry`;
CREATE TABLE `spawn_group_entry`  (
  `Id` int(11) NOT NULL COMMENT 'Spawn Group ID',
  `Entry` int(11) NOT NULL COMMENT 'Entry of creature or GO',
  `MinCount` int(11) NOT NULL DEFAULT 0 COMMENT 'Minimum count of entry in a group before random',
  `MaxCount` int(11) NOT NULL DEFAULT 0 COMMENT 'Maximum total count of entry in a group',
  `Chance` int(11) NOT NULL DEFAULT 0 COMMENT 'Chance for entry to be selected',
  PRIMARY KEY (`Id`, `Entry`)
);

-- ----------------------------
-- Table structure for spawn_group_formation
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group_formation`;
CREATE TABLE `spawn_group_formation`  (
  `Id` int(11) NOT NULL COMMENT 'Spawn group id',
  `FormationType` tinyint(11) NOT NULL DEFAULT 0 COMMENT 'Formation shape 0..6',
  `FormationSpread` float(11, 0) NOT NULL DEFAULT 0 COMMENT 'Distance between formation members',
  `FormationOptions` int(11) NOT NULL DEFAULT 0 COMMENT 'Keep formation compact (bit 1)',
  `PathId` int(11) NOT NULL DEFAULT 0 COMMENT 'PathId from waypoint_path path',
  `MovementType` tinyint(11) NOT NULL COMMENT 'Same as creature table',
  `Comment` varchar(255) NULL DEFAULT NULL,
  PRIMARY KEY (`Id`)
);

-- ----------------------------
-- Table structure for spawn_group_linked_group
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group_linked_group`;
CREATE TABLE `spawn_group_linked_group`  (
  `Id` int(11) NOT NULL COMMENT 'Spawn Group ID',
  `LinkedId` int(11) NOT NULL COMMENT 'Linked Spawn Group ID',
  PRIMARY KEY (`Id`, `LinkedId`)
);

-- ----------------------------
-- Table structure for spawn_group_squad
-- ----------------------------

DROP TABLE IF EXISTS spawn_group_squad;
CREATE TABLE spawn_group_squad(
Id INT NOT NULL COMMENT 'Spawn Group ID',
SquadId INT NOT NULL COMMENT 'Squad Id within Spawn Group',
Guid INT NOT NULL COMMENT 'Guid of creature or GO',
Entry INT NOT NULL COMMENT 'Entry of creature or GO',
PRIMARY KEY(Id, SquadId, Guid)
);

--
-- Table structure for table `spam_records`
--

CREATE TABLE IF NOT EXISTS `spam_records` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `record` VARCHAR(512) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='REGEX Spam records';

--
-- Table structure for table `spell_affect`
--

DROP TABLE IF EXISTS `spell_affect`;
CREATE TABLE `spell_affect` (
  `entry` smallint(5) unsigned NOT NULL DEFAULT '0',
  `effectId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`effectId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_affect`
--

LOCK TABLES `spell_affect` WRITE;
/*!40000 ALTER TABLE `spell_affect` DISABLE KEYS */;
INSERT INTO `spell_affect` VALUES
(11083,0,0x0000000000C20017),
(11115,0,0x0000000008C20017),
(11124,0,0x0000000008C20017),
(11170,0,0x0000000040FE1AF7),
(11242,0,0x0000000000001000),
(11247,0,0x0000000000002000),
(11367,0,0x0000000008C20017),
(11368,0,0x0000000008C20017),
(11369,0,0x0000000008C20017),
(11370,0,0x0000000008C20017),
(12042,0,0x0000000020E01AF7),
(12042,1,0x0000000020E01AF7),
(12042,2,0x0000000020E01AF7),
(12285,0,0x0000000000000001),
(12288,0,0x0000000000000008),
(12288,1,0x0000000000000008),
(12301,0,0x0000000000000100),
(12351,0,0x0000000000C20017),
(12378,0,0x0000000008C20017),
(12398,0,0x0000000008C20017),
(12399,0,0x0000000008C20017),
(12400,0,0x0000000008C20017),
(12467,0,0x0000000000001000),
(12469,0,0x0000000000001000),
(12536,0,0x0000000020C01AF7),
(12593,1,0x0000000020E01AF7),
(12697,0,0x0000000000000001),
(12707,0,0x0000000000000008),
(12707,1,0x0000000000000008),
(12818,0,0x0000000000000100),
(12842,0,0x0000000000240000),
(12982,0,0x0000000040FE1AF7),
(12983,0,0x0000000040FE1AF7),
(12984,0,0x0000000040FE1AF7),
(12985,0,0x0000000040FE1AF7),
(13742,0,0x0000000000000060),
(13743,0,0x0000000000000040),
(13872,0,0x0000000000000060),
(13875,0,0x0000000000000040),
(13975,1,0x0000000000400000),
(13976,0,0x0000000000000700),
(13979,0,0x0000000000000700),
(13980,0,0x0000000000000700),
(13981,0,0x0000000001000800),
(14062,1,0x0000000000400000),
(14063,1,0x0000000000400000),
(14064,1,0x0000000000400000),
(14065,1,0x0000000000400000),
(14066,0,0x0000000001000800),
(14067,0,0x0000000001000800),
(14082,0,0x0000000000000500),
(14083,0,0x0000000000000500),
(14113,0,0x000000001001E000),
(14114,0,0x000000001001E000),
(14115,0,0x000000001001E000),
(14116,0,0x000000001001E000),
(14117,0,0x000000001001E000),
(14143,0,0x0000000004000206),
(14149,0,0x0000000004000206),
(14168,0,0x0000000000080000),
(14169,0,0x0000000000080000),
(14174,0,0x0000000000200000),
(14175,0,0x0000000000200000),
(14176,0,0x0000000000200000),
(14909,1,0x0000000000100080),
(15017,1,0x0000000000100080),
(15058,0,0x00000000002212E0),
(15059,0,0x00000000002212E0),
(15060,0,0x00000000002212E0),
(16035,0,0x0000000090100003),
(16038,0,0x0000000040000000),
(16039,0,0x0000000090100003),
(16041,0,0x0000000000000003),
(16043,0,0x0000000000000008),
(16043,1,0x0000000020000000),
(16086,0,0x0000000000000020),
(16086,1,0x0000000008000000),
(16086,2,0x0000000000000004),
(16089,0,0x00000004D3300407),
(16105,0,0x0000000090100003),
(16106,0,0x0000000090100003),
(16106,1,0x0000000010000000),
(16107,0,0x0000000090100003),
(16108,0,0x0000000090100003),
(16109,0,0x0000000090100003),
(16110,0,0x0000000090100003),
(16111,0,0x0000000090100003),
(16112,0,0x0000000090100003),
(16117,0,0x0000000000000003),
(16118,0,0x0000000000000003),
(16119,0,0x0000000000000003),
(16120,0,0x0000000000000003),
(16130,0,0x0000000000000008),
(16130,1,0x0000000020000000),
(16160,0,0x0000000040000000),
(16161,0,0x0000000040000000),
(16166,0,0x0000000090100003),
(16166,1,0x0000000090100003),
(16173,0,0x0000000020080000),
(16181,0,0x00000000000001C0),
(16183,0,0x0000000000000100),
(16184,0,0x0000000000000200),
(16184,1,0x0000000000000200),
(16187,0,0x0000000000006000),
(16189,0,0x000000000403E000),
(16205,0,0x0000000000006000),
(16206,0,0x0000000000006000),
(16207,0,0x0000000000006000),
(16208,0,0x0000000000006000),
(16209,1,0x0000000000000200),
(16222,0,0x0000000020080000),
(16223,0,0x0000000020080000),
(16224,0,0x0000000020080000),
(16225,0,0x0000000020080000),
(16230,0,0x00000000000001C0),
(16232,0,0x00000000000001C0),
(16233,0,0x00000000000001C0),
(16234,0,0x00000000000001C0),
(16258,0,0x0000000000008000),
(16258,1,0x0000000000040000),
(16259,0,0x0000000000010000),
(16259,1,0x0000000000020000),
(16266,0,0x0000000000400000),
(16266,1,0x0000000001000000),
(16266,2,0x0000000000800000),
(16293,0,0x0000000000008000),
(16293,1,0x0000000000040000),
(16295,0,0x0000000000010000),
(16295,1,0x0000000000020000),
(16513,0,0x000000080001E000),
(16513,1,0x000000000001E000),
(16513,2,0x000000000001E000),
(16514,0,0x000000080001E000),
(16514,1,0x000000000001E000),
(16514,2,0x000000000001E000),
(16515,0,0x000000080001E000),
(16515,1,0x000000000001E000),
(16515,2,0x000000000001E000),
(16544,2,0x0000000000000004),
(16719,0,0x000000080001E000),
(16719,1,0x000000000001E000),
(16719,2,0x000000000001E000),
(16720,0,0x000000080001E000),
(16720,1,0x000000000001E000),
(16720,2,0x000000000001E000),
(16870,0,0x001007F100E3BEFF),
(17123,0,0x0000000000000080),
(17124,0,0x0000000000000080),
(17904,0,0x0000000000000000),
(17912,0,0x0000000000000000),
(17913,0,0x0000000000000000),
(17914,0,0x0000000000000000),
(17915,0,0x0000000000000000),
(17916,0,0x0000000000000000),
(17954,1,0x000000C000001364),
(17955,1,0x000000C000001364),
(17956,1,0x000000C000001364),
(17957,1,0x000000C000001364),
(17958,1,0x000000C000001364),
(18174,0,0x00000001804D841A),
(18175,0,0x00000001804D841A),
(18176,0,0x00000001804D841A),
(18177,0,0x00000001804D841A),
(18178,0,0x00000001804D841A),
(18218,0,0x00000001804D841A),
(18219,0,0x00000001804D841A),
(18271,0,0x0000000300016489),
(18271,1,0x000000010008040A),
(18271,2,0x0000000000000000),
(18272,0,0x0000000300016489),
(18272,1,0x000000010008040A),
(18272,2,0x0000000000000000),
(18273,0,0x0000000300016489),
(18273,1,0x000000010008040A),
(18273,2,0x0000000000000000),
(18274,0,0x0000000300016489),
(18274,1,0x000000010008040A),
(18274,2,0x0000000000000000),
(18275,0,0x0000000300016489),
(18275,1,0x000000010008040A),
(18275,2,0x0000000000000000),
(20101,0,0x000000000A800200),
(20102,0,0x000000000A800200),
(20103,0,0x000000000A800200),
(20104,0,0x000000000A800200),
(20105,0,0x000000000A800200),
(20575,1,0x0000000004000000),
(21942,1,0x0000000008000000),
(23300,0,0x0000000020000000),
(26118,0,0x0000000004000000),
(26118,1,0x0000000004000000),
(28539,0,0x0000000000001000),
(28682,0,0x0000000000400017),
(28743,0,0x00000000000000F0),
(28746,1,0x0000000000010406),
(28751,0,0x0000000000021000),
(28755,0,0x0000000000000020),
(28763,0,0x0000000004000000),
(28774,0,0x0000000000008000),
(28787,0,0x0000000200000000),
(28807,0,0x0000000000000040),
(28808,0,0x0000000411041E40),
(28811,0,0x0000000000800000),
(28814,0,0x0000000000020000),
(28815,0,0x0000000002000006),
(28818,0,0x0000000020080000),
(28821,0,0x0000000000000400),
(28829,0,0x0000000000000002),
(28830,0,0x0000000000040000),
(28831,0,0x0000000000000001),
(28842,0,0x0000000100004440),
(28843,0,0x0000000000088000),
(28844,0,0x0000000000000400),
(28852,0,0x0000000000000200),
(28852,1,0x0000000020000000),
(28855,0,0x0000000000000800),
(28999,0,0x0000000000000003),
(29000,0,0x0000000000000003),
(29005,0,0x0000000090100003),
(29063,0,0x00000000000009C3),
(29079,0,0x0000000000400000),
(29079,1,0x0000000001000000),
(29079,2,0x0000000000800000),
(29080,0,0x0000000000400000),
(29080,1,0x0000000001000000),
(29080,2,0x0000000000800000),
(29171,0,0x0000000000004000),
(29187,0,0x00000000000001C0),
(29189,0,0x00000000000001C0),
(29191,0,0x00000000000001C0),
(29192,0,0x0000000000800000),
(29192,1,0x0000000000200000),
(29193,0,0x0000000000800000),
(29193,1,0x0000000000200000),
(29202,0,0x0000000000000040),
(29205,0,0x0000000000000040),
(29206,0,0x0000000000000040),
(29438,0,0x00000000004002F7),
(29439,0,0x00000000004002F7),
(29440,0,0x00000000004002F7),
(30440,0,0x0000000000088000),
(30441,0,0x0000000800000000),
(30640,0,0x0000000000000100),
(30812,0,0x00000000B0181418),
(30813,0,0x00000000B0181418),
(30814,0,0x00000000B0181418),
(30872,0,0x0000000000000100),
(30892,0,0x0000000008000000),
(30893,0,0x0000000008000000);
/*!40000 ALTER TABLE `spell_affect` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_area`
--

DROP TABLE IF EXISTS `spell_area`;
CREATE TABLE `spell_area` (
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `area` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest_start` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest_start_active` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `quest_end` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `condition_id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `aura_spell` mediumint(8) NOT NULL DEFAULT '0',
  `racemask` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(1) unsigned NOT NULL DEFAULT '2',
  `autocast` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spell`,`area`,`quest_start`,`quest_start_active`,`aura_spell`,`racemask`,`gender`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_area`
--

LOCK TABLES `spell_area` WRITE;
/*!40000 ALTER TABLE `spell_area` DISABLE KEYS */;
/*!40000 ALTER TABLE `spell_area` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_bonus_data`
--

DROP TABLE IF EXISTS `spell_bonus_data`;
CREATE TABLE `spell_bonus_data` (
  `entry` smallint(5) unsigned NOT NULL,
  `direct_bonus` float NOT NULL DEFAULT '0',
  `dot_bonus` float NOT NULL DEFAULT '0',
  `ap_bonus` float NOT NULL DEFAULT '0',
  `ap_dot_bonus` float NOT NULL DEFAULT '0',
  `comments` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_bonus_data`
--

LOCK TABLES `spell_bonus_data` WRITE;
/*!40000 ALTER TABLE `spell_bonus_data` DISABLE KEYS */;
INSERT INTO `spell_bonus_data` VALUES
/* Druid */
(339,   0,      0.1,     0,     0,     'Druid - Entangling Roots'),
(5185,  1.6104, 0,       0,     0,     'Druid - Healing Touch'),
(5570,  0,      0.127,   0,     0,     'Druid - Insect Swarm'),
(8921,  0.1515, 0.13,    0,     0,     'Druid - Moonfire'),
(8936,  0.3,    0.1,     0,     0,     'Druid - Regrowth'),
(18562, 0,      0,       0,     0,     'Druid - Swiftmend'),
(5176,  0.5714, 0,       0,     0,     'Druid - Wrath'),
/* Mage */
(7268,  0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 1'),
(7269,  0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 2'),
(7270,  0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 3'),
(8419,  0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 4'),
(8418,  0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 5'),
(10273, 0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 6'),
(10274, 0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 7'),
(25346, 0.2857, 0,       0,     0,     'Mage - Arcane Missiles Triggered Spell Rank 8'),
(11113, 0.1357, 0,       0,     0,     'Mage - Blast Wave Rank'),
(120,   0.1357, 0,       0,     0,     'Mage - Cone of Cold'),
(133,   1,      0,       0,     0,     'Mage - Fire Ball'),
(2120,  0.2357, 0.122,   0,     0,     'Mage - Flamestrike'),
(116,   0.8143, 0,       0,     0,     'Mage - Frost Bolt'),
(1463,  0.8053, 0,       0,     0,     'Mage - Mana Shield'),
/* Paladin */
(20911, 0,      0,       0,     0,     'Paladin - Blessing of Sanctuary'),
(25997, 1,      0,       0,     0,     'Paladin - Eye for an Eye'),
(25899, 0,      0,       0,     0,     'Paladin - Greater Blessing of Sanctuary'),
(19968, 0.7143, 0,       0,     0,     'Paladin - Holy Light'),
(20925, 0.05,   0,       0,     0,     'Paladin - Holy Shield'),
(2812,  0.1905, 0,       0,     0,     'Paladin - Holy Wrath'),
(20187, 0.5,    0,       0,     0,     'Paladin - Judgement of Righteousness'),
(20424, 0.2,    0,       0,     0,     'Paladin - Seal of Command Proc'),
(25742, 0.1,    0,       0,     0,     'Paladin - Seal of Righteousness Proc'),
/* Priest */
(2061,  0.6177, 0,       0,     0,     'Priest - Flash Heal'),
(2060,  1.2353, 0,       0,     0,     'Priest - Greater Heal'),
(14914, 0.5711, 0.024,   0,     0,     'Priest - Holy Fire'),
(15237, 0.1606, 0,       0,     0,     'Priest - Holy Nova Damage'),
(23455, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 1'),
(23458, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 2'),
(23459, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 3'),
(27803, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 4'),
(27804, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 5'),
(27805, 0.3035, 0,       0,     0,     'Priest - Holy Nova Heal Rank 6'),
(8129,  0,      0,       0,     0,     'Priest - Mana Burn'),
(15407, 0,      0.19,    0,     0,     'Priest - Mind Flay'),
(585,   0.714,  0,       0,     0,     'Priest - Smite'),
/* Rogue */
(703,   0,      0,       0,     0.03,  'Rogue - Garrote'),
/* Shaman */
(421,   0.57,   0,       0,     0,     'Shaman - Chain Lightning'),
(974,   0.2857, 0,       0,     0,     'Shaman - Earth Shield'),
(379,   0,      0,       0,     0,     'Shaman - Earth Shield Triggered'),
(8042,  0.3858, 0,       0,     0,     'Shaman - Earth Shock'),
(8443,  0.2142, 0,       0,     0,     'Shaman - Fire Nova Totem Casted by Totem Rank 1'),
(8504,  0.2142, 0,       0,     0,     'Shaman - Fire Nova Totem Casted by Totem Rank 2'),
(8505,  0.2142, 0,       0,     0,     'Shaman - Fire Nova Totem Casted by Totem Rank 3'),
(11310, 0.2142, 0,       0,     0,     'Shaman - Fire Nova Totem Casted by Totem Rank 4'),
(11311, 0.2142, 0,       0,     0,     'Shaman - Fire Nova Totem Casted by Totem Rank 5'),
(8050,  0.2142, 0.1,     0,     0,     'Shaman - Flame Shock'),
(10444, 0,      0,       0,     0,     'Shaman - Flametongue Attack'),
(8026,  0.1,    0,       0,     0,     'Shaman - Flametongue Weapon Proc Rank 1'),
(8056,  0.3858, 0,       0,     0,     'Shaman - Frost Shock'),
(8034,  0.1,    0,       0,     0,     'Shaman - Frostbrand Attack Rank 1'),
(5672,  0,      0.0450,  0,     0,     'Shaman - Healing Stream Totem'),
(331,   0.8571, 0,       0,     0,     'Shaman - Healing Wave'),
(403,   0.7143, 0,       0,     0,     'Shaman - Lightning Bolt'),
(26364, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 1'),
(26365, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 2'),
(26366, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 3'),
(26367, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 4'),
(26369, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 5'),
(26370, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 6'),
(26363, 0.33,   0,       0,     0,     'Shaman - Lightning Shield Proc Rank 7'),
(8188,  0.1,    0,       0,     0,     'Shaman - Magma Totam Passive Rank 1'),
(10582, 0.1,    0,       0,     0,     'Shaman - Magma Totam Passive Rank 2'),
(10583, 0.1,    0,       0,     0,     'Shaman - Magma Totam Passive Rank 3'),
(10584, 0.1,    0,       0,     0,     'Shaman - Magma Totam Passive Rank 4'),
(3606,  0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 1'),
(6350,  0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 2'),
(6351,  0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 3'),
(6352,  0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 4'),
(10435, 0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 5'),
(10436, 0.1667, 0,       0,     0,     'Shaman - Searing Totem Attack Rank 6'),
/* Warlock */
(172,   0,      0.156,   0,     0,     'Warlock - Corruption'),
(980,   0,      0.1,     0,     0,     'Warlock - Curse of Agony'),
(603,   0,      2,       0,     0,     'Warlock - Curse of Doom'),
(18220, 0.96,   0,       0,     0,     'Warlock - Dark Pact'),
(5138,  0,      0,       0,     0,     'Warlock - Drain Mana'),
(1120,  0,      0.4286,  0,     0,     'Warlock - Drain Soul'),
(18790, 0,      0,       0,     0,     'Warlock - Fel Stamina'),
(755,   0,      0.4485,  0,     0,     'Warlock - Health Funnel'),
(1949,  0,      0.0946,  0,     0,     'Warlock - Hellfire'),
(5857,  0.1428, 0,       0,     0,     'Warlock - Hellfire Effect on Enemy Rank 1'),
(11681, 0.1428, 0,       0,     0,     'Warlock - Hellfire Effect on Enemy Rank 2'),
(11682, 0.1428, 0,       0,     0,     'Warlock - Hellfire Effect on Enemy Rank 3'),
(348,   0.2,    0.2,     0,     0,     'Warlock - Immolate'),
(1454,  0.8,    0,       0,     0,     'Warlock - Life Tap'),
(686,   0.8571, 0,       0,     0,     'Warlock - Shadow Bolt'),
(6229,  0.3,    0,       0,     0,     'Warlock - Shadow Ward'),
(6353,  1.15,   0,       0,     0,     'Warlock - Soul Fire'),
/* Item */
(18764, 0,      0,       0,     0,     'Item - Fungal Regrowth'),
(17712, 0,      0,       0,     0,     'Item - Lifestone Healing'),
(5707,  0,      0,       0,     0,     'Item - Lifestone Regeneration'),
(21179, 0,      0,       0,     0,     'Item - Six Demon Bag - Chain Lightning'),
(15662, 0,      0,       0,     0,     'Item - Six Demon Bag - Fireball'),
(11538, 0,      0,       0,     0,     'Item - Six Demon Bag - Frostbolt');
/*!40000 ALTER TABLE `spell_bonus_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_chain`
--

DROP TABLE IF EXISTS `spell_chain`;
CREATE TABLE `spell_chain` (
  `spell_id` mediumint(9) NOT NULL DEFAULT '0',
  `prev_spell` mediumint(9) NOT NULL DEFAULT '0',
  `first_spell` mediumint(9) NOT NULL DEFAULT '0',
  `rank` tinyint(4) NOT NULL DEFAULT '0',
  `req_spell` mediumint(9) NOT NULL DEFAULT '0',
  PRIMARY KEY (`spell_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Spell Additinal Data';

--
-- Dumping data for table `spell_chain`
--

LOCK TABLES `spell_chain` WRITE;
/*!40000 ALTER TABLE `spell_chain` DISABLE KEYS */;
INSERT INTO `spell_chain` VALUES
-- ------------------
-- (0) Not associated with skills
-- ------------------
/* Deadly Poison Triggered */
(2818,0,2818,1,0),
(2819,2818,2818,2,0),
(11353,2819,2818,3,0),
(11354,11353,2818,4,0),
(25349,11354,2818,5,0),
/* Enrage Triggered, Warrior */
(12880,0,12880,1,0),
(14201,12880,12880,2,0),
(14202,14201,12880,3,0),
(14203,14202,12880,4,0),
(14204,14203,12880,5,0),
/* Fire Resistance Totem Auras */
(8185,0,8185,1,0),
(10534,8185,8185,2,0),
(10535,10534,8185,3,0),
/* Flametongue Weapon Proc */
(8026, 0, 8026, 1, 0),
(8028, 8026, 8026, 2, 0),
(8029, 8028, 8026, 3, 0),
(10445, 8029, 8026, 4, 0),
(16343, 10445, 8026, 5, 0),
(16344, 16343, 8026, 6, 0),
/* Flurry triggered, Warrior */
(12966,0,12966,1,0),
(12967,12966,12966,2,0),
(12968,12967,12966,3,0),
(12969,12968,12966,4,0),
(12970,12969,12966,5,0),
/* Flurry triggered, Shaman */
(16257,0,16257,1,0),
(16277,16257,16257,2,0),
(16278,16277,16257,3,0),
(16279,16278,16257,4,0),
(16280,16279,16257,5,0),
/* Frostbrand Attack */
(8034,0,8034,1,0),
(8037,8034,8034,2,0),
(10458,8037,8034,3,0),
(16352,10458,8034,4,0),
(16353,16352,8034,5,0),
/* Frost Resistance Totem Auras */
(8182,0,8182,1,0),
(10476,8182,8182,2,0),
(10477,10476,8182,3,0),
/* Grace of Air Totem Auras */
(8836,0,8836,1,0),
(10626,8836,8836,2,0),
(25360,10626,8836,3,0),
/* Healing Stream Totem Auras */
(5672,0,5672, 1,0),
(6371,5672,5672,2,0),
(6372,6371,5672,3,0),
(10460,6372,5672,4,0),
(10461,10460,5672,5,0),
/* Instant Poison */
(8680,0,8680,1,0),
(8685,8680,8680,2,0),
(8689,8685,8680,3,0),
(11335,8689,8680,4,0),
(11336,11335,8680,5,0),
(11337,11336,8680,6,0),
/* Nature Resistance Totem Auras */
(10596,0,10596,1,0),
(10598,10596,10596,2,0),
(10599,10598,10596,3,0),
/* Stoneskin Totem Auras */
(8072,0,8072,1,0),
(8156,8072,8072,2,0),
(8157,8156,8072,3,0),
(10403,8157,8072,4,0),
(10404,10403,8072,5,0),
(10405,10404,8072,6,0),
/* Strength of Earth Totem Auras */
(8076,0,8076,1,0),
(8162,8076,8076,2,0),
(8163,8162,8076,3,0),
(10441,8163,8076,4,0),
(25362,10441,8076,5,0),
/* Windwall Totem Auras */
(15108,0,15108,1,0),
(15109,15108,15108,2,0),
(15110,15109,15108,3,0),
/* Wound Poison */
(13218,0,13218,1,0),
(13222,13218,13218,2,0),
(13223,13222,13218,3,0),
(13224,13223,13218,4,0),
-- ------------------
-- (6) Frost
-- ------------------
/* Cone of Cold */
(120,0,120,1,0),
(8492,120,120,2,0),
(10159,8492,120,3,0),
(10160,10159,120,4,0),
(10161,10160,120,5,0),
/* Blizzard */
(10,0,10,1,0),
(6141,10,10,2,0),
(8427,6141,10,3,0),
(10185,8427,10,4,0),
(10186,10185,10,5,0),
(10187,10186,10,6,0),
/* Frost Armor */
(168,0,168,1,0),
(7300,168,168,2,0),
(7301,7300,168,3,0),
/* Frostbolt */
(116,0,116,1,0),
(205,116,116,2,0),
(837,205,116,3,0),
(7322,837,116,4,0),
(8406,7322,116,5,0),
(8407,8406,116,6,0),
(8408,8407,116,7,0),
(10179,8408,116,8,0),
(10180,10179,116,9,0),
(10181,10180,116,10,0),
(25304,10181,116,11,0),
/* Frost Nova */
(122,0,122,1,0),
(865,122,122,2,0),
(6131,865,122,3,0),
(10230,6131,122,4,0),
/* Frost Ward */
(6143,0,6143,1,0),
(8461,6143,6143,2,0),
(8462,8461,6143,3,0),
(10177,8462,6143,4,0),
(28609,10177,6143,5,0),
/* Ice Armor */
(7302,0,7302,1,0),
(7320,7302,7302,2,0),
(10219,7320,7302,3,0),
(10220,10219,7302,4,0),
/* Ice Barrier */
(11426,0,11426,1,0),
(13031,11426,11426,2,0),
(13032,13031,11426,3,0),
(13033,13032,11426,4,0),
-- ------------------
-- (8)Fire
-- ------------------
/* Blast Wave */
(11113,0,11113,1,0),
(13018,11113,11113,2,0),
(13019,13018,11113,3,0),
(13020,13019,11113,4,0),
(13021,13020,11113,5,0),
/* Fire Blast */
(2136,0,2136,1,0),
(2137,2136,2136,2,0),
(2138,2137,2136,3,0),
(8412,2138,2136,4,0),
(8413,8412,2136,5,0),
(10197,8413,2136,6,0),
(10199,10197,2136,7,0),
/* Fireball */
(133,0,133,1,0),
(143,133,133,2,0),
(145,143,133,3,0),
(3140,145,133,4,0),
(8400,3140,133,5,0),
(8401,8400,133,6,0),
(8402,8401,133,7,0),
(10148,8402,133,8,0),
(10149,10148,133,9,0),
(10150,10149,133,10,0),
(10151,10150,133,11,0),
(25306,10151,133,12,0),
/* Fire Ward */
(543,0,543,1,0),
(8457,543,543,2,0),
(8458,8457,543,3,0),
(10223,8458,543,4,0),
(10225,10223,543,5,0),
/* Flamestrike */
(2120,0,2120,1,0),
(2121,2120,2120,2,0),
(8422,2121,2120,3,0),
(8423,8422,2120,4,0),
(10215,8423,2120,5,0),
(10216,10215,2120,6,0),
/* Pyroblast */
(11366,0,11366,1,0),
(12505,11366,11366,2,0),
(12522,12505,11366,3,0),
(12523,12522,11366,4,0),
(12524,12523,11366,5,0),
(12525,12524,11366,6,0),
(12526,12525,11366,7,0),
(18809,12526,11366,8,0),
/* Scorch */
(2948,0,2948,1,0),
(8444,2948,2948,2,0),
(8445,8444,2948,3,0),
(8446,8445,2948,4,0),
(10205,8446,2948,5,0),
(10206,10205,2948,6,0),
(10207,10206,2948,7,0),
-- ------------------
-- (40) Poisons
-- ------------------
/* Crippling Poison */
(3420,0,3420,1,0),
(3421,3420,3420,2,0),
/* Deadly Poison */
(2835,0,2835,1,0),
(2837,2835,2835,2,0),
(11357,2837,2835,3,0),
(11358,11357,2835,4,0),
(25347,11358,2835,5,0),
/* Instant Poison */
(8681,0,8681,1,0),
(8687,8681,8681,2,0),
(8691,8687,8681,3,0),
(11341,8691,8681,4,0),
(11342,11341,8681,5,0),
(11343,11342,8681,6,0),
/* Mind-numbing Poison */
(5763,0,5763,1,0),
(8694,5763,5763,2,0),
(11400,8694,5763,3,0),
/* Wound Poison */
(13220,0,13220,1,0),
(13228,13220,13220,2,0),
(13229,13228,13220,3,0),
(13230,13229,13220,4,0),
-- ------------------
-- (50) Beast Mastery
-- ------------------
/* Aspect of the Hawk */
(13165,0,13165,1,0),
(14318,13165,13165,2,0),
(14319,14318,13165,3,0),
(14320,14319,13165,4,0),
(14321,14320,13165,5,0),
(14322,14321,13165,6,0),
(25296,14322,13165,7,0),
/* Aspect of the Wild */
(20043,0,20043,1,0),
(20190,20043,20043,2,0),
/* Mend Pet */
(136,0,136,1,0),
(3111,136,136,2,0),
(3661,3111,136,3,0),
(3662,3661,136,4,0),
(13542,3662,136,5,0),
(13543,13542,136,6,0),
(13544,13543,136,7,0),
/* Scare Beast */
(1513,0,1513,1,0),
(14326,1513,1513,2,0),
(14327,14326,1513,3,0),
-- ------------------
-- (51)Survival
-- ------------------
/* Counterattack */
(19306,0,19306,1,0),
(20909,19306,19306,2,0),
(20910,20909,19306,3,0),
/* Disengage */
(781,0,781,1,0),
(14272,781,781,2,0),
(14273,14272,781,3,0),
/* Explosive Trap */
(13813,0,13813,1,0),
(14316,13813,13813,2,0),
(14317,14316,13813,3,0),
/* Explosive Trap Effect */
(13812,0,13812,1,0),
(14314,13812,13812,2,0),
(14315,14314,13812,3,0),
/* Freezing Trap */
(1499,0,1499,1,0),
(14310,1499,1499,2,0),
(14311,14310,1499,3,0),
/* Immolation Trap */
(13795,0,13795,1,0),
(14302,13795,13795,2,0),
(14303,14302,13795,3,0),
(14304,14303,13795,4,0),
(14305,14304,13795,5,0),
/* Immolation Trap Triggered */
(13797,0,13797,1,0),
(14298,13797,13797,2,0),
(14299,14298,13797,3,0),
(14300,14299,13797,4,0),
(14301,14300,13797,5,0),
/* Mongoose Bite */
(1495,0,1495,1,0),
(14269,1495,1495,2,0),
(14270,14269,1495,3,0),
(14271,14270,1495,4,0),
/* Raptor Strike */
(2973,0,2973,1,0),
(14260,2973,2973,2,0),
(14261,14260,2973,3,0),
(14262,14261,2973,4,0),
(14263,14262,2973,5,0),
(14264,14263,2973,6,0),
(14265,14264,2973,7,0),
(14266,14265,2973,8,0),
/* Wing Clip */
(2974,0,2974,1,0),
(14267,2974,2974,2,0),
(14268,14267,2974,3,0),
/* Wyvern Sting */
(19386,0,19386,1,0),
(24132,19386,19386,2,0),
(24133,24132,19386,3,0),
-- ------------------
-- (56) Holy (Priest)
-- ------------------
/* Desperate Prayer */
(13908,0,13908,1,0),
(19236,13908,13908,2,0),
(19238,19236,13908,3,0),
(19240,19238,13908,4,0),
(19241,19240,13908,5,0),
(19242,19241,13908,6,0),
(19243,19242,13908,7,0),
/* Flash Heal */
(2061,0,2061,1,0),
(9472,2061,2061,2,0),
(9473,9472,2061,3,0),
(9474,9473,2061,4,0),
(10915,9474,2061,5,0),
(10916,10915,2061,6,0),
(10917,10916,2061,7,0),
/* Greater Heal */
(2060,0,2060,1,0),
(10963,2060,2060,2,0),
(10964,10963,2060,3,0),
(10965,10964,2060,4,0),
(25314,10965,2060,5,0),
/* Heal */
(2054,0,2054,1,0),
(2055,2054,2054,2,0),
(6063,2055,2054,3,0),
(6064,6063,2054,4,0),
/* Holy Fire */
(14914,0,14914,1,0),
(15262,14914,14914,2,0),
(15263,15262,14914,3,0),
(15264,15263,14914,4,0),
(15265,15264,14914,5,0),
(15266,15265,14914,6,0),
(15267,15266,14914,7,0),
(15261,15267,14914,8,0),
/* Holy Nova */
(15237,0,15237,1,0),
(15430,15237,15237,2,0),
(15431,15430,15237,3,0),
(27799,15431,15237,4,0),
(27800,27799,15237,5,0),
(27801,27800,15237,6,0),
/* Lesser Heal */
(2050,0,2050,1,0),
(2052,2050,2050,2,0),
(2053,2052,2050,3,0),
/* Lightwell */
(724,0,724,1,0),
(27870,724,724,2,0),
(27871,27870,724,3,0),
/* Prayer of Healing */
(596,0,596,1,0),
(996,596,596,2,0),
(10960,996,596,3,0),
(10961,10960,596,4,0),
(25316,10961,596,5,0),
/* Renew */
(139,0,139,1,0),
(6074,139,139,2,0),
(6075,6074,139,3,0),
(6076,6075,139,4,0),
(6077,6076,139,5,0),
(6078,6077,139,6,0),
(10927,6078,139,7,0),
(10928,10927,139,8,0),
(10929,10928,139,9,0),
(25315,10929,139,10,0),
/* Resurrection */
(2006,0,2006,1,0),
(2010,2006,2006,2,0),
(10880,2010,2006,3,0),
(10881,10880,2006,4,0),
(20770,10881,2006,5,0),
/* Smite */
(585,0,585,1,0),
(591,585,585,2,0),
(598,591,585,3,0),
(984,598,585,4,0),
(1004,984,585,5,0),
(6060,1004,585,6,0),
(10933,6060,585,7,0),
(10934,10933,585,8,0),
-- ------------------
-- (78) Shadow Magic
-- ------------------
/* Devouring Plague */
(2944,0,2944,1,0),
(19276,2944,2944,2,0),
(19277,19276,2944,3,0),
(19278,19277,2944,4,0),
(19279,19278,2944,5,0),
(19280,19279,2944,6,0),
/* Fade */
(586,0,586,1,0),
(9578,586,586,2,0),
(9579,9578,586,3,0),
(9592,9579,586,4,0),
(10941,9592,586,5,0),
(10942,10941,586,6,0),
/* Mind Control */
(605,0,605,1,0),
(10911,605,605,2,0),
(10912,10911,605,3,0),
/* Hex of Weakness */
(9035,0,9035,1,0),
(19281,9035,9035,2,0),
(19282,19281,9035,3,0),
(19283,19282,9035,4,0),
(19284,19283,9035,5,0),
(19285,19284,9035,6,0),
/* Mind Blast */
(8092,0,8092,1,0),
(8102,8092,8092,2,0),
(8103,8102,8092,3,0),
(8104,8103,8092,4,0),
(8105,8104,8092,5,0),
(8106,8105,8092,6,0),
(10945,8106,8092,7,0),
(10946,10945,8092,8,0),
(10947,10946,8092,9,0),
/* Mind Flay */
(15407,0,15407,1,0),
(17311,15407,15407,2,0),
(17312,17311,15407,3,0),
(17313,17312,15407,4,0),
(17314,17313,15407,5,0),
(18807,17314,15407,6,0),
/* Mind Soothe */
(453,0,453,1,0),
(8192,453,453,2,0),
(10953,8192,453,3,0),
/* Mind Vision */
(2096,0,2096,1,0),
(10909,2096,2096,2,0),
/* Psychic Scream */
(8122,0,8122,1,0),
(8124,8122,8122,2,0),
(10888,8124,8122,3,0),
(10890,10888,8122,4,0),
/* Shadowguard */
(18137,0,18137,1,0),
(19308,18137,18137,2,0),
(19309,19308,18137,3,0),
(19310,19309,18137,4,0),
(19311,19310,18137,5,0),
(19312,19311,18137,6,0),
/* Shadow Protection */
(976,0,976,1,0),
(10957,976,976,2,0),
(10958,10957,976,3,0),
/* Shadow Word: Pain */
(589,0,589,1,0),
(594,589,589,2,0),
(970,594,589,3,0),
(992,970,589,4,0),
(2767,992,589,5,0),
(10892,2767,589,6,0),
(10893,10892,589,7,0),
(10894,10893,589,8,0),
/* Touch of Weakness */
(2652,0,2652,1,0),
(19261,2652,2652,2,0),
(19262,19261,2652,3,0),
(19264,19262,2652,4,0),
(19265,19264,2652,5,0),
(19266,19265,2652,6,0),
-- ------------------
-- (134) Feral Combat (Druid)
-- ------------------
/* Faerie Fire (Feral) */
(16857,0,16857,1,0),
(17390,16857,16857,2,0),
(17391,17390,16857,3,0),
(17392,17391,16857,4,0),
-- ------------------
-- (163)Marksmanship
-- ------------------
/* Aimed Shot */
(19434,0,19434,1,0),
(20900,19434,19434,2,0),
(20901,20900,19434,3,0),
(20902,20901,19434,4,0),
(20903,20902,19434,5,0),
(20904,20903,19434,6,0),
/* Arcane Shot */
(3044,0,3044,1,0),
(14281,3044,3044,2,0),
(14282,14281,3044,3,0),
(14283,14282,3044,4,0),
(14284,14283,3044,5,0),
(14285,14284,3044,6,0),
(14286,14285,3044,7,0),
(14287,14286,3044,8,0),
/* Distracting Shot */
(20736,0,20736,1,0),
(14274,20736,20736,2,0),
(15629,14274,20736,3,0),
(15630,15629,20736,4,0),
(15631,15630,20736,5,0),
(15632,15631,20736,6,0),
/* Hunter's Mark */
(1130,0,1130,1,0),
(14323,1130,1130,2,0),
(14324,14323,1130,3,0),
(14325,14324,1130,4,0),
/* Multi-Shot */
(2643,0,2643,1,0),
(14288,2643,2643,2,0),
(14289,14288,2643,3,0),
(14290,14289,2643,4,0),
(25294,14290,2643,5,0),
/* Serpent Sting */
(1978,0,1978,1,0),
(13549,1978,1978,2,0),
(13550,13549,1978,3,0),
(13551,13550,1978,4,0),
(13552,13551,1978,5,0),
(13553,13552,1978,6,0),
(13554,13553,1978,7,0),
(13555,13554,1978,8,0),
(25295,13555,1978,9,0),
/* Trueshot Aura */
(19506,0,19506,1,0),
(20905,19506,19506,2,0),
(20906,20905,19506,3,0),
/* Viper Sting */
(3034,0,3034,1,0),
(14279,3034,3034,2,0),
(14280,14279,3034,3,0),
/* Volley */
(1510,0,1510,1,0),
(14294,1510,1510,2,0),
(14295,14294,1510,3,0),
-- ------------------
-- (164) Blacksmithing
-- ------------------
/* Blacksmithing */
(9787,9785,2018,5,0),
(9788,9785,2018,5,0),
(17039,9787,2018,6,0),
(17040,9787,2018,6,0),
(17041,9787,2018,6,0),
-- ------------------
-- (165) Leatherworking
-- ------------------
/* Leatherworking */
(10656,10662,2108,5,0),
(10658,10662,2108,5,0),
(10660,10662,2108,5,0),
-- ------------------
-- (184) Retribution (Paladin)
-- ------------------
/* Blessing of Might */
(19740,0,19740,1,0),
(19834,19740,19740,2,0),
(19835,19834,19740,3,0),
(19836,19835,19740,4,0),
(19837,19836,19740,5,0),
(19838,19837,19740,6,0),
(25291,19838,19740,7,0),
/* Greater Blessing of Might */
(25782,0,25782,1,19838),
(25916,25782,25782,2,25291),
/* Hammer of Wrath */
(24275,0,24275,1,0),
(24274,24275,24275,2,0),
(24239,24274,24275,3,0),
/* Seal of Command */
(20375,0,20375,1,0),
(20915,20375,20375,2,0),
(20918,20915,20375,3,0),
(20919,20918,20375,4,0),
(20920,20919,20375,5,0),
/* Seal of the Crusader */
(21082,0,21082,1,0),
(20162,21082,21082,2,0),
(20305,20162,21082,3,0),
(20306,20305,21082,4,0),
(20307,20306,21082,5,0),
(20308,20307,21082,6,0),
-- ------------------
-- (237)Arcane
-- ------------------
/* Amplify Magic */
(1008,0,1008,1,0),
(8455,1008,1008,2,0),
(10169,8455,1008,3,0),
(10170,10169,1008,4,0),
/* Arcane Explosion */
(1449,0,1449,1,0),
(8437,1449,1449,2,0),
(8438,8437,1449,3,0),
(8439,8438,1449,4,0),
(10201,8439,1449,5,0),
(10202,10201,1449,6,0),
/* Arcane Intellect */
(1459,0,1459,1,0),
(1460,1459,1459,2,0),
(1461,1460,1459,3,0),
(10156,1461,1459,4,0),
(10157,10156,1459,5,0),
/* Arcane Missiles */
(5143,0,5143,1,0),
(5144,5143,5143,2,0),
(5145,5144,5143,3,0),
(8416,5145,5143,4,0),
(8417,8416,5143,5,0),
(10211,8417,5143,6,0),
(10212,10211,5143,7,0),
(25345,10212,5143,8,0),
/* Conjure Food */
(587,0,587,1,0),
(597,587,587,2,0),
(990,597,587,3,0),
(6129,990,587,4,0),
(10144,6129,587,5,0),
(10145,10144,587,6,0),
(28612,10145,587,7,0),
/* Conjure Mana Gem */
(759,0,759,1,0),
(3552,759,759,2,0),
(10053,3552,759,3,0),
(10054,10053,759,4,0),
/* Conjure Water */
(5504,0,5504,1,0),
(5505,5504,5504,2,0),
(5506,5505,5504,3,0),
(6127,5506,5504,4,0),
(10138,6127,5504,5,0),
(10139,10138,5504,6,0),
(10140,10139,5504,7,0),
/* Dampen Magic */
(604,0,604,1,0),
(8450,604,604,2,0),
(8451,8450,604,3,0),
(10173,8451,604,4,0),
(10174,10173,604,5,0),
/* Mage Armor */
(6117,0,6117,1,0),
(22782,6117,6117,2,0),
(22783,22782,6117,3,0),
/* Mana Shield */
(1463,0,1463,1,0),
(8494,1463,1463,2,0),
(8495,8494,1463,3,0),
(10191,8495,1463,4,0),
(10192,10191,1463,5,0),
(10193,10192,1463,6,0),
/* Polymorph */
(118,0,118,1,0),
(12824,118,118,2,0),
(12825,12824,118,3,0),
(12826,12825,118,4,0),
-- ------------------
-- (267) Protection (Paladin)
-- ------------------
/* Blessing of Kings */
(20217,0,20217,1,0),
/* Blessing of Protection */
(1022,0,1022,1,0),
(5599,1022,1022,2,0),
(10278,5599,1022,3,0),
/* Blessing of Sacrifice */
(6940,0,6940,1,0),
(20729,6940,6940,2,0),
/* Blessing of Salvation */
(1038,0,1038,1,0),
/* Blessing of Sanctuary */
(20911,0,20911,1,0),
(20912,20911,20911,2,0),
(20913,20912,20911,3,0),
(20914,20913,20911,4,0),
/* Divine Protection */
(498,0,498,1,0),
(5573,498,498,2,0),
/* Divine Shield */
(642,0,642,1,0),
(1020,642,642,2,0),
/* Greater Blessing of Kings */
(25898,0,25898,1,20217),
/* Greater Blessing of Salvation */
(25895,0,25895,1,1038),
/* Greater Blessing of Sanctuary */
(25899,0,25899,1,20914),
/* Hammer of Justice */
(853,0,853,1,0),
(5588,853,853,2,0),
(5589,5588,853,3,0),
(10308,5589,853,4,0),
/* Holy Shield */
(20925,0,20925,1,0),
(20927,20925,20925,2,0),
(20928,20927,20925,3,0),
-- ------------------
-- (354)Demonology
-- ------------------
/* Banish */
(710,0,710,1,0),
(18647,710,710,2,0),
/* Create Firestone */
(6366,0,6366,1,0),
(17951,6366,6366,2,0),
(17952,17951,6366,3,0),
(17953,17952,6366,4,0),
/* Create Healthstone */
(6201,0,6201,1,0),
(6202,6201,6201,2,0),
(5699,6202,6201,3,0),
(11729,5699,6201,4,0),
(11730,11729,6201,5,0),
/* Create Soulstone */
(693,0,693,1,0),
(20752,693,693,2,0),
(20755,20752,693,3,0),
(20756,20755,693,4,0),
(20757,20756,693,5,0),
/* Create Spellstone */
(2362,0,2362,1,0),
(17727,2362,2362,2,0),
(17728,17727,2362,3,0),
/* Demon Armor */
(706,0,706,1,0),
(1086,706,706,2,0),
(11733,1086,706,3,0),
(11734,11733,706,4,0),
(11735,11734,706,5,0),
/* Demon Skin */
(687,0,687,1,0),
(696,687,687,2,0),
/* Enslave Demon */
(1098,0,1098,1,0),
(11725,1098,1098,2,0),
(11726,11725,1098,3,0),
/* Health Funnel */
(755,0,755,1,0),
(3698,755,755,2,0),
(3699,3698,755,3,0),
(3700,3699,755,4,0),
(11693,3700,755,5,0),
(11694,11693,755,6,0),
(11695,11694,755,7,0),
/* Shadow Ward */
(6229,0,6229,1,0),
(11739,6229,6229,2,0),
(11740,11739,6229,3,0),
(28610,11740,6229,4,0),
-- ------------------
-- (355)Affliction
-- ------------------
/* Corruption */
(172,0,172,1,0),
(6222,172,172,2,0),
(6223,6222,172,3,0),
(7648,6223,172,4,0),
(11671,7648,172,5,0),
(11672,11671,172,6,0),
(25311,11672,172,7,0),
/* Curse of Agony */
(980,0,980,1,0),
(1014,980,980,2,0),
(6217,1014,980,3,0),
(11711,6217,980,4,0),
(11712,11711,980,5,0),
(11713,11712,980,6,0),
/* Curse of the Elements */
(1490,0,1490,1,0),
(11721,1490,1490,2,0),
(11722,11721,1490,3,0),
/* Curse of Recklessness */
(704,0,704,1,0),
(7658,704,704,2,0),
(7659,7658,704,3,0),
(11717,7659,704,4,0),
/* Curse of Shadow */
(17862,0,17862,1,0),
(17937,17862,17862,2,0),
/* Curse of Tongues */
(1714,0,1714,1,0),
(11719,1714,1714,2,0),
/* Curse of Weakness */
(702,0,702,1,0),
(1108,702,702,2,0),
(6205,1108,702,3,0),
(7646,6205,702,4,0),
(11707,7646,702,5,0),
(11708,11707,702,6,0),
/* Dark Pact */
(18220,0,18220,1,0),
(18937,18220,18220,2,0),
(18938,18937,18220,3,0),
/* Death Coil */
(6789,0,6789,1,0),
(17925,6789,6789,2,0),
(17926,17925,6789,3,0),
/* Drain Life */
(689,0,689,1,0),
(699,689,689,2,0),
(709,699,689,3,0),
(7651,709,689,4,0),
(11699,7651,689,5,0),
(11700,11699,689,6,0),
/* Drain Mana */
(5138,0,5138,1,0),
(6226,5138,5138,2,0),
(11703,6226,5138,3,0),
(11704,11703,5138,4,0),
/* Drain Soul */
(1120,0,1120,1,0),
(8288,1120,1120,2,0),
(8289,8288,1120,3,0),
(11675,8289,1120,4,0),
/* Fear */
(5782,0,5782,1,0),
(6213,5782,5782,2,0),
(6215,6213,5782,3,0),
/* Howl of Terror */
(5484,0,5484,1,0),
(17928,5484,5484,2,0),
/* Life Tap */
(1454,0,1454,1,0),
(1455,1454,1454,2,0),
(1456,1455,1454,3,0),
(11687,1456,1454,4,0),
(11688,11687,1454,5,0),
(11689,11688,1454,6,0),
/* Siphon Life */
(18265,0,18265,1,0),
(18879,18265,18265,2,0),
(18880,18879,18265,3,0),
(18881,18880,18265,4,0),
-- ------------------
-- (373) Enhancement
-- ------------------
/* Fire Resistance Totem */
(8184,0,8184,1,0),
(10537,8184,8184,2,0),
(10538,10537,8184,3,0),
/* Flametongue Totem */
(8227,0,8227,1,0),
(8249,8227,8227,2,0),
(10526,8249,8227,3,0),
(16387,10526,8227,4,0),
/* Flametongue Weapon */
(8024,0,8024,1,0),
(8027,8024,8024,2,0),
(8030,8027,8024,3,0),
(16339,8030,8024,4,0),
(16341,16339,8024,5,0),
(16342,16341,8024,6,0),
/* Frost Resistance Totem */
(8181,0,8181,1,0),
(10478,8181,8181,2,0),
(10479,10478,8181,3,0),
/* Frostbrand Weapon */
(8033,0,8033,1,0),
(8038,8033,8033,2,0),
(10456,8038,8033,3,0),
(16355,10456,8033,4,0),
(16356,16355,8033,5,0),
/* Grace of Air Totem */
(8835,0,8835,1,0),
(10627,8835,8835,2,0),
(25359,10627,8835,3,0),
/* Lightning Shield */
(324,0,324,1,0),
(325,324,324,2,0),
(905,325,324,3,0),
(945,905,324,4,0),
(8134,945,324,5,0),
(10431,8134,324,6,0),
(10432,10431,324,7,0),
/* Nature Resistance Totem */
(10595,0,10595,1,0),
(10600,10595,10595,2,0),
(10601,10600,10595,3,0),
/* Rockbiter Weapon */
(8017,0,8017,1,0),
(8018,8017,8017,2,0),
(8019,8018,8017,3,0),
(10399,8019,8017,4,0),
(16314,10399,8017,5,0),
(16315,16314,8017,6,0),
(16316,16315,8017,7,0),
/* Stoneskin Totem */
(8071,0,8071,1,0),
(8154,8071,8071,2,0),
(8155,8154,8071,3,0),
(10406,8155,8071,4,0),
(10407,10406,8071,5,0),
(10408,10407,8071,6,0),
/* Strength of Earth Totem */
(8075,0,8075,1,0),
(8160,8075,8075,2,0),
(8161,8160,8075,3,0),
(10442,8161,8075,4,0),
(25361,10442,8075,5,0),
/* Windfury Totem */
(8512,0,8512,1,0),
(10613,8512,8512,2,0),
(10614,10613,8512,3,0),
/* Windfury Weapon */
(8232,0,8232,1,0),
(8235,8232,8232,2,0),
(10486,8235,8232,3,0),
(16362,10486,8232,4,0),
/* Windwall Totem */
(15107,0,15107,1,0),
(15111,15107,15107,2,0),
(15112,15111,15107,3,0),
-- ------------------
-- (374) Restoration (Shaman)
-- ------------------
/* Ancestral Spirit */
(2008,0,2008,1,0),
(20609,2008,2008,2,0),
(20610,20609,2008,3,0),
(20776,20610,2008,4,0),
(20777,20776,2008,5,0),
/* Chain Heal */
(1064,0,1064,1,0),
(10622,1064,1064,2,0),
(10623,10622,1064,3,0),
/* Entangling Roots */
(339,0,339,1,0),
(1062,339,339,2,0),
(5195,1062,339,3,0),
(5196,5195,339,4,0),
(9852,5196,339,5,0),
(9853,9852,339,6,0),
/* Healing Stream Totem */
(5394,0,5394,1,0),
(6375,5394,5394,2,0),
(6377,6375,5394,3,0),
(10462,6377,5394,4,0),
(10463,10462,5394,5,0),
/* Healing Wave */
(331,0,331,1,0),
(332,331,331,2,0),
(547,332,331,3,0),
(913,547,331,4,0),
(939,913,331,5,0),
(959,939,331,6,0),
(8005,959,331,7,0),
(10395,8005,331,8,0),
(10396,10395,331,9,0),
(25357,10396,331,10,0),
/* Lesser Healing Wave */
(8004,0,8004,1,0),
(8008,8004,8004,2,0),
(8010,8008,8004,3,0),
(10466,8010,8004,4,0),
(10467,10466,8004,5,0),
(10468,10467,8004,6,0),
/* Mana Spring Totem */
(5675,0,5675,1,0),
(10495,5675,5675,2,0),
(10496,10495,5675,3,0),
(10497,10496,5675,4,0),
-- ------------------
-- (375) Elemental Combat
-- ------------------
/* Chain Lightning */
(421,0,421,1,0),
(930,421,421,2,0),
(2860,930,421,3,0),
(10605,2860,421,4,0),
/* Earth Shock */
(8042,0,8042,1,0),
(8044,8042,8042,2,0),
(8045,8044,8042,3,0),
(8046,8045,8042,4,0),
(10412,8046,8042,5,0),
(10413,10412,8042,6,0),
(10414,10413,8042,7,0),
/* Fire Nova */
(1535,0,1535,1,0),
(8498,1535,1535,2,0),
(8499,8498,1535,3,0),
(11314,8499,1535,4,0),
(11315,11314,1535,5,0),
/* Flame Shock */
(8050,0,8050,1,0),
(8052,8050,8050,2,0),
(8053,8052,8050,3,0),
(10447,8053,8050,4,0),
(10448,10447,8050,5,0),
(29228,10448,8050,6,0),
/* Frost Shock */
(8056,0,8056,1,0),
(8058,8056,8056,2,0),
(10472,8058,8056,3,0),
(10473,10472,8056,4,0),
/* Lightning Bolt */
(403,0,403,1,0),
(529,403,403,2,0),
(548,529,403,3,0),
(915,548,403,4,0),
(943,915,403,5,0),
(6041,943,403,6,0),
(10391,6041,403,7,0),
(10392,10391,403,8,0),
(15207,10392,403,9,0),
(15208,15207,403,10,0),
/* Magma Totem */
(8190,0,8190,1,0),
(10585,8190,8190,2,0),
(10586,10585,8190,3,0),
(10587,10586,8190,4,0),
/* Purge */
(370,0,370,1,0),
(8012,370,370,2,0),
/* Searing Totem */
(3599,0,3599,1,0),
(6363,3599,3599,2,0),
(6364,6363,3599,3,0),
(6365,6364,3599,4,0),
(10437,6365,3599,5,0),
(10438,10437,3599,6,0),
/* Stoneclaw Totem */
(5730,0,5730,1,0),
(6390,5730,5730,2,0),
(6391,6390,5730,3,0),
(6392,6391,5730,4,0),
(10427,6392,5730,5,0),
(10428,10427,5730,6,0),
-- ------------------
-- (573)Restoration
-- ------------------
/* Gift of the Wild */
(21849,0,21849,1,0),
(21850,21849,21849,2,0),
/* Healing Touch */
(5185,0,5185,1,0),
(5186,5185,5185,2,0),
(5187,5186,5185,3,0),
(5188,5187,5185,4,0),
(5189,5188,5185,5,0),
(6778,5189,5185,6,0),
(8903,6778,5185,7,0),
(9758,8903,5185,8,0),
(9888,9758,5185,9,0),
(9889,9888,5185,10,0),
(25297,9889,5185,11,0),
/* Mark of the Wild */
(1126,0,1126,1,0),
(5232,1126,1126,2,0),
(6756,5232,1126,3,0),
(5234,6756,1126,4,0),
(8907,5234,1126,5,0),
(9884,8907,1126,6,0),
(9885,9884,1126,7,0),
/* Rebirth */
(20484,0,20484,1,0),
(20739,20484,20484,2,0),
(20742,20739,20484,3,0),
(20747,20742,20484,4,0),
(20748,20747,20484,5,0),
/* Regrowth */
(8936,0,8936,1,0),
(8938,8936,8936,2,0),
(8939,8938,8936,3,0),
(8940,8939,8936,4,0),
(8941,8940,8936,5,0),
(9750,8941,8936,6,0),
(9856,9750,8936,7,0),
(9857,9856,8936,8,0),
(9858,9857,8936,9,0),
/* Rejuvenation */
(774,0,774,1,0),
(1058,774,774,2,0),
(1430,1058,774,3,0),
(2090,1430,774,4,0),
(2091,2090,774,5,0),
(3627,2091,774,6,0),
(8910,3627,774,7,0),
(9839,8910,774,8,0),
(9840,9839,774,9,0),
(9841,9840,774,10,0),
(25299,9841,774,11,0),
/* Tranquility */
(740,0,740,1,0),
(8918,740,740,2,0),
(9862,8918,740,3,0),
(9863,9862,740,4,0),
-- ------------------
-- (574)Balance
-- ------------------
/* Faerie Fire */
(770,0,770,1,0),
(778,770,770,2,0),
(9749,778,770,3,0),
(9907,9749,770,4,0),
/* Hibernate */
(2637,0,2637,1,0),
(18657,2637,2637,2,0),
(18658,18657,2637,3,0),
/* Hurricane */
(16914,0,16914,1,0),
(17401,16914,16914,2,0),
(17402,17401,16914,3,0),
/* Insect Swarm */
(5570,0,5570,1,0),
(24974,5570,5570,2,0),
(24975,24974,5570,3,0),
(24976,24975,5570,4,0),
(24977,24976,5570,5,0),
/* Moonfire */
(8921,0,8921,1,0),
(8924,8921,8921,2,0),
(8925,8924,8921,3,0),
(8926,8925,8921,4,0),
(8927,8926,8921,5,0),
(8928,8927,8921,6,0),
(8929,8928,8921,7,0),
(9833,8929,8921,8,0),
(9834,9833,8921,9,0),
(9835,9834,8921,10,0),
/* Nature'sGrasp */
(16689,0,16689,1,339),
(16810,16689,16689,2,1062),
(16811,16810,16689,3,5195),
(16812,16811,16689,4,5196),
(16813,16812,16689,5,9852),
(17329,16813,16689,6,9853),
/* Soothe Animal */
(2908,0,2908,1,0),
(8955,2908,2908,2,0),
(9901,8955,2908,3,0),
/* Starfire */
(2912,0,2912,1,0),
(8949,2912,2912,2,0),
(8950,8949,2912,3,0),
(8951,8950,2912,4,0),
(9875,8951,2912,5,0),
(9876,9875,2912,6,0),
(25298,9876,2912,7,0),
/* Thorns */
(467,0,467,1,0),
(782,467,467,2,0),
(1075,782,467,3,0),
(8914,1075,467,4,0),
(9756,8914,467,5,0),
(9910,9756,467,6,0),
/* Wrath */
(5176,0,5176,1,0),
(5177,5176,5176,2,0),
(5178,5177,5176,3,0),
(5179,5178,5176,4,0),
(5180,5179,5176,5,0),
(6780,5180,5176,6,0),
(8905,6780,5176,7,0),
(9912,8905,5176,8,0),
-- ------------------
-- (593)Destruction
-- ------------------
/* Conflagrate */
(17962,0,17962,1,0),
(18930,17962,17962,2,0),
(18931,18930,17962,3,0),
(18932,18931,17962,4,0),
/* Hellfire */
(1949,0,1949,1,0),
(11683,1949,1949,2,0),
(11684,11683,1949,3,0),
/* Immolate */
(348,0,348,1,0),
(707,348,348,2,0),
(1094,707,348,3,0),
(2941,1094,348,4,0),
(11665,2941,348,5,0),
(11667,11665,348,6,0),
(11668,11667,348,7,0),
(25309,11668,348,8,0),
/* Rain of Fire */
(5740,0,5740,1,0),
(6219,5740,5740,2,0),
(11677,6219,5740,3,0),
(11678,11677,5740,4,0),
/* Searing Pain */
(5676,0,5676,1,0),
(17919,5676,5676,2,0),
(17920,17919,5676,3,0),
(17921,17920,5676,4,0),
(17922,17921,5676,5,0),
(17923,17922,5676,6,0),
/* Shadow Bolt */
(686,0,686,1,0),
(695,686,686,2,0),
(705,695,686,3,0),
(1088,705,686,4,0),
(1106,1088,686,5,0),
(7641,1106,686,6,0),
(11659,7641,686,7,0),
(11660,11659,686,8,0),
(11661,11660,686,9,0),
(25307,11661,686,10,0),
/* Shadowburn */
(17877,0,17877,1,0),
(18867,17877,17877,2,0),
(18868,18867,17877,3,0),
(18869,18868,17877,4,0),
(18870,18869,17877,5,0),
(18871,18870,17877,6,0),
/* Soul Fire */
(6353,0,6353,1,0),
(17924,6353,6353,2,0),
-- ------------------
-- (594) Holy (Paladin)
-- ------------------
/* Blessing of Light */
(19977,0,19977,1,0),
(19978,19977,19977,2,0),
(19979,19978,19977,3,0),
/* Blessing of Wisdom */
(19742,0,19742,1,0),
(19850,19742,19742,2,0),
(19852,19850,19742,3,0),
(19853,19852,19742,4,0),
(19854,19853,19742,5,0),
(25290,19854,19742,6,0),
/* Consecration */
(26573,0,26573,1,0),
(20116,26573,26573,2,0),
(20922,20116,26573,3,0),
(20923,20922,26573,4,0),
(20924,20923,26573,5,0),
/* Exorcism */
(879,0,879,1,0),
(5614,879,879,2,0),
(5615,5614,879,3,0),
(10312,5615,879,4,0),
(10313,10312,879,5,0),
(10314,10313,879,6,0),
/* Flash of Light */
(19750,0,19750,1,0),
(19939,19750,19750,2,0),
(19940,19939,19750,3,0),
(19941,19940,19750,4,0),
(19942,19941,19750,5,0),
(19943,19942,19750,6,0),
/* Greater Blessing of Light */
(25890,0,25890,1,19979),
/* Greater Blessing of Wisdom */
(25894,0,25894,1,19854),
(25918,25894,25894,2,25290),
/* Holy Light */
(635,0,635,1,0),
(639,635,635,2,0),
(647,639,635,3,0),
(1026,647,635,4,0),
(1042,1026,635,5,0),
(3472,1042,635,6,0),
(10328,3472,635,7,0),
(10329,10328,635,8,0),
(25292,10329,635,9,0),
/* Holy Shock */
(20473,0,20473,1,0),
(20929,20473,20473,2,0),
(20930,20929,20473,3,0),
/* Holy Wrath */
(2812,0,2812,1,0),
(10318,2812,2812,2,0),
/* Judgement of Righteousness */
(20187,0,20187,1,0),
(20280,20187,20187,2,0),
(20281,20280,20187,3,0),
(20282,20281,20187,4,0),
(20283,20282,20187,5,0),
(20284,20283,20187,6,0),
(20285,20284,20187,7,0),
(20286,20285,20187,8,0),
/* Redemption */
(7328,0,7328,1,0),
(10322,7328,7328,2,0),
(10324,10322,7328,3,0),
(20772,10324,7328,4,0),
(20773,20772,7328,5,0),
/* Seal of Light */
(20165,0,20165,1,0),
(20347,20165,20165,2,0),
(20348,20347,20165,3,0),
(20349,20348,20165,4,0),
/* Seal of Righteousness (serverside extension) */
(20287,21084,20154,3,0),
(20288,20287,20154,4,0),
(20289,20288,20154,5,0),
(20290,20289,20154,6,0),
(20291,20290,20154,7,0),
(20292,20291,20154,8,0),
(20293,20292,20154,9,0),
/* Seal of Righteousness Proc */
(25742,0,25742,1,0),
(25741,25742,25742,2,0),
(25740,25741,25742,3,0),
(25739,25740,25742,4,0),
(25738,25739,25742,5,0),
(25737,25738,25742,6,0),
(25736,25737,25742,7,0),
(25735,25736,25742,8,0),
(25713,25735,25742,9,0),
/* Seal of Wisdom */
(20166,0,20166,1,0),
(20356,20166,20166,2,0),
(20357,20356,20166,3,0),
/* Turn Undead */
(2878,0,2878,1,0),
(5627,2878,2878,2,0),
(10326,5627,2878,3,0),
-- ------------------
-- (613)Discipline
-- ------------------
/* Dispel Magic*/
(527,0,527,1,0),
(988,527,527,2,0),
/* Divine Spirit */
(14752,0,14752,1,0),
(14818,14752,14752,2,0),
(14819,14818,14752,3,0),
(27841,14819,14752,4,0),
/* Elune's Grace */
(2651,0,2651,1,0),
(19289,2651,2651,2,0),
(19291,19289,2651,3,0),
(19292,19291,2651,4,0),
(19293,19292,2651,5,0),
/* Feedback */
(13896,0,13896,1,0),
(19271,13896,13896,2,0),
(19273,19271,13896,3,0),
(19274,19273,13896,4,0),
(19275,19274,13896,5,0),
/* InnerFire */
(588,0,588,1,0),
(7128,588,588,2,0),
(602,7128,588,3,0),
(1006,602,588,4,0),
(10951,1006,588,5,0),
(10952,10951,588,6,0),
/* Mana Burn */
(8129,0,8129,1,0),
(8131,8129,8129,2,0),
(10874,8131,8129,3,0),
(10875,10874,8129,4,0),
(10876,10875,8129,5,0),
/* Power Word: Fortitude */
(1243,0,1243,1,0),
(1244,1243,1243,2,0),
(1245,1244,1243,3,0),
(2791,1245,1243,4,0),
(10937,2791,1243,5,0),
(10938,10937,1243,6,0),
/* PowerWord:Shield */
(17,0,17,1,0),
(592,17,17,2,0),
(600,592,17,3,0),
(3747,600,17,4,0),
(6065,3747,17,5,0),
(6066,6065,17,6,0),
(10898,6066,17,7,0),
(10899,10898,17,8,0),
(10900,10899,17,9,0),
(10901,10900,17,10,0),
/* Prayer of Fortitude */
(21562,0,21562,1,0),
(21564,21562,21562,2,0),
/* Prayer of Spirit */
(27681,14752,14752,2,0),
/* Shackle Undead */
(9484,0,9484,1,0),
(9485,9484,9484,2,0),
(10955,9485,9484,3,0),
/* Starshards */
(10797,0,10797,1,0),
(19296,10797,10797,2,0),
(19299,19296,10797,3,0),
(19302,19299,10797,4,0),
(19303,19302,10797,5,0),
(19304,19303,10797,6,0),
(19305,19304,10797,7,0),
-- ------------------
-- (762)Riding
-- ------------------
/*Riding*/
(33388,0,33388,1,0),
(33391,33388,33388,2,0);
/*!40000 ALTER TABLE `spell_chain` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_elixir`
--

DROP TABLE IF EXISTS `spell_elixir`;
CREATE TABLE `spell_elixir` (
  `entry` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'SpellId of potion',
  `mask` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'Mask 0x1 battle 0x2 guardian 0x3 flask 0x7 unstable flasks 0xB shattrath flasks',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Spell System';

--
-- Dumping data for table `spell_elixir`
--

LOCK TABLES `spell_elixir` WRITE;
/*!40000 ALTER TABLE `spell_elixir` DISABLE KEYS */;
INSERT INTO `spell_elixir` VALUES
(  673,0x2),
( 2367,0x1),
( 2374,0x1),
( 2378,0x2),
( 2380,0x2),
( 3160,0x1),
( 3164,0x1),
( 3166,0x2),
( 3219,0x2),
( 3220,0x2),
( 3222,0x2),
( 3223,0x2),
( 3593,0x2),
( 7844,0x1),
( 8212,0x1),
(10667,0x1),
(10668,0x2),
(10669,0x1),
(10692,0x2),
(10693,0x2),
(11319,0x2),
(11328,0x1),
(11334,0x1),
(11348,0x2),
(11349,0x2),
(11364,0x2),
(11371,0x2),
(11390,0x1),
(11396,0x2),
(11405,0x1),
(11406,0x1),
(11474,0x1),
(15231,0x2),
(15233,0x2),
(16321,0x2),
(16322,0x1),
(16323,0x1),
(16325,0x2),
(16326,0x2),
(16327,0x2),
(16329,0x1),
(17038,0x1),
(17535,0x2),
(17537,0x1),
(17538,0x1),
(17539,0x1),
(17624,0x3),
(17626,0x3),
(17627,0x3),
(17628,0x3),
(17629,0x3),
(21920,0x1),
(24361,0x2),
(24363,0x2),
(24382,0x2),
(24383,0x2),
(24417,0x2),
(26276,0x1),
(27652,0x2),
(27653,0x2);
/*!40000 ALTER TABLE `spell_elixir` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_facing`
--

DROP TABLE IF EXISTS `spell_facing`;
CREATE TABLE `spell_facing` (
  `entry` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell ID',
  `facingcasterflag` tinyint(1) NOT NULL DEFAULT '1' COMMENT 'flag for facing state, usually 1',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `spell_facing`
--

LOCK TABLES `spell_facing` WRITE;
/*!40000 ALTER TABLE `spell_facing` DISABLE KEYS */;
INSERT INTO `spell_facing` VALUES
(53,1),
(72,1),
(75,1),
(78,1),
(100,1),
(116,1),
(133,1),
(143,1),
(145,1),
(205,1),
(284,1),
(285,1),
(348,1),
(403,1),
(408,1),
(421,1),
(529,1),
(548,1),
(585,1),
(591,1),
(598,1),
(676,1),
(686,1),
(689,1),
(694,1),
(695,1),
(699,1),
(703,1),
(705,1),
(707,1),
(709,1),
(769,1),
(772,1),
(779,1),
(780,1),
(837,1),
(845,1),
(879,1),
(915,1),
(930,1),
(943,1),
(984,1),
(1004,1),
(1079,1),
(1082,1),
(1088,1),
(1094,1),
(1106,1),
(1120,1),
(1329,1),
(1464,1),
(1495,1),
(1608,1),
(1671,1),
(1672,1),
(1715,1),
(1742,1),
(1752,1),
(1753,1),
(1754,1),
(1755,1),
(1756,1),
(1757,1),
(1758,1),
(1759,1),
(1760,1),
(1766,1),
(1767,1),
(1768,1),
(1769,1),
(1776,1),
(1777,1),
(1822,1),
(1823,1),
(1824,1),
(1833,1),
(1943,1),
(1966,1),
(1978,1),
(2070,1),
(2098,1),
(2136,1),
(2137,1),
(2138,1),
(2589,1),
(2590,1),
(2591,1),
(2643,1),
(2764,1),
(2860,1),
(2912,1),
(2941,1),
(2948,1),
(2973,1),
(2974,1),
(3009,1),
(3010,1),
(3018,1),
(3029,1),
(3034,1),
(3043,1),
(3044,1),
(3140,1),
(3674,1),
(4164,1),
(4165,1),
(5019,1),
(5116,1),
(5133,1),
(5138,1),
(5143,1),
(5144,1),
(5145,1),
(5176,1),
(5177,1),
(5178,1),
(5179,1),
(5180,1),
(5201,1),
(5211,1),
(5221,1),
(5306,1),
(5308,1),
(5375,1),
(5401,1),
(5614,1),
(5615,1),
(5676,1),
(5938,1),
(6041,1),
(6060,1),
(6178,1),
(6252,1),
(6254,1),
(6255,1),
(6353,1),
(6546,1),
(6547,1),
(6548,1),
(6552,1),
(6554,1),
(6555,1),
(6572,1),
(6574,1),
(6660,1),
(6726,1),
(6760,1),
(6761,1),
(6762,1),
(6768,1),
(6770,1),
(6780,1),
(6785,1),
(6787,1),
(6789,1),
(6798,1),
(6800,1),
(6807,1),
(6808,1),
(6809,1),
(6949,1),
(7105,1),
(7145,1),
(7268,1),
(7269,1),
(7270,1),
(7290,1),
(7295,1),
(7322,1),
(7369,1),
(7372,1),
(7373,1),
(7379,1),
(7384,1),
(7386,1),
(7400,1),
(7402,1),
(7405,1),
(7641,1),
(7651,1),
(7712,1),
(7714,1),
(7715,1),
(7716,1),
(7717,1),
(7718,1),
(7719,1),
(7769,1),
(7887,1),
(8042,1),
(8044,1),
(8045,1),
(8046,1),
(8050,1),
(8052,1),
(8053,1),
(8056,1),
(8058,1),
(8092,1),
(8102,1),
(8103,1),
(8104,1),
(8105,1),
(8106,1),
(8129,1),
(8246,1),
(8255,1),
(8288,1),
(8289,1),
(8380,1),
(8398,1),
(8400,1),
(8401,1),
(8402,1),
(8406,1),
(8407,1),
(8408,1),
(8412,1),
(8413,1),
(8416,1),
(8417,1),
(8418,1),
(8419,1),
(8444,1),
(8445,1),
(8446,1),
(8598,1),
(8621,1),
(8623,1),
(8624,1),
(8629,1),
(8631,1),
(8632,1),
(8633,1),
(8637,1),
(8639,1),
(8640,1),
(8643,1),
(8647,1),
(8649,1),
(8650,1),
(8676,1),
(8682,1),
(8721,1),
(8724,1),
(8725,1),
(8820,1),
(8905,1),
(8921,1),
(8924,1),
(8925,1),
(8926,1),
(8927,1),
(8928,1),
(8929,1),
(8949,1),
(8950,1),
(8951,1),
(8972,1),
(8983,1),
(8992,1),
(8998,1),
(9000,1),
(9005,1),
(9053,1),
(9057,1),
(9081,1),
(9373,1),
(9481,1),
(9483,1),
(9487,1),
(9488,1),
(9492,1),
(9493,1),
(9532,1),
(9591,1),
(9613,1),
(9654,1),
(9672,1),
(9734,1),
(9739,1),
(9745,1),
(9752,1),
(9754,1),
(9771,1),
(9823,1),
(9827,1),
(9829,1),
(9830,1),
(9833,1),
(9834,1),
(9835,1),
(9849,1),
(9850,1),
(9866,1),
(9867,1),
(9875,1),
(9876,1),
(9880,1),
(9881,1),
(9892,1),
(9894,1),
(9896,1),
(9904,1),
(9908,1),
(9912,1),
(10148,1),
(10149,1),
(10150,1),
(10151,1),
(10179,1),
(10180,1),
(10181,1),
(10197,1),
(10199,1),
(10205,1),
(10206,1),
(10207,1),
(10211,1),
(10212,1),
(10273,1),
(10274,1),
(10277,1),
(10312,1),
(10313,1),
(10314,1),
(10346,1),
(10391,1),
(10392,1),
(10412,1),
(10413,1),
(10414,1),
(10447,1),
(10448,1),
(10472,1),
(10473,1),
(10578,1),
(10605,1),
(10933,1),
(10934,1),
(10945,1),
(10946,1),
(10947,1),
(11021,1),
(11131,1),
(11197,1),
(11198,1),
(11267,1),
(11268,1),
(11269,1),
(11273,1),
(11274,1),
(11275,1),
(11279,1),
(11280,1),
(11281,1),
(11285,1),
(11286,1),
(11289,1),
(11290,1),
(11293,1),
(11294,1),
(11297,1),
(11299,1),
(11300,1),
(11303,1),
(11366,1),
(11538,1),
(11564,1),
(11565,1),
(11566,1),
(11567,1),
(11572,1),
(11573,1),
(11574,1),
(11578,1),
(11584,1),
(11585,1),
(11596,1),
(11597,1),
(11600,1),
(11601,1),
(11604,1),
(11605,1),
(11608,1),
(11609,1),
(11659,1),
(11660,1),
(11661,1),
(11665,1),
(11667,1),
(11668,1),
(11675,1),
(11699,1),
(11700,1),
(11839,1),
(11921,1),
(11985,1),
(11988,1),
(11989,1),
(12058,1),
(12167,1),
(12280,1),
(12294,1),
(12466,1),
(12471,1),
(12505,1),
(12522,1),
(12523,1),
(12524,1),
(12525,1),
(12526,1),
(12540,1),
(12555,1),
(12557,1),
(12611,1),
(12675,1),
(12693,1),
(12737,1),
(12739,1),
(12809,1),
(13140,1),
(13322,1),
(13339,1),
(13340,1),
(13341,1),
(13342,1),
(13374,1),
(13375,1),
(13398,1),
(13438,1),
(13439,1),
(13440,1),
(13441,1),
(13480,1),
(13482,1),
(13491,1),
(13519,1),
(13527,1),
(13549,1),
(13550,1),
(13551,1),
(13552,1),
(13553,1),
(13554,1),
(13555,1),
(13579,1),
(13748,1),
(13900,1),
(13901,1),
(13953,1),
(14034,1),
(14105,1),
(14106,1),
(14109,1),
(14112,1),
(14119,1),
(14122,1),
(14145,1),
(14200,1),
(14251,1),
(14260,1),
(14261,1),
(14262,1),
(14263,1),
(14264,1),
(14265,1),
(14266,1),
(14269,1),
(14270,1),
(14271,1),
(14278,1),
(14281,1),
(14282,1),
(14283,1),
(14284,1),
(14285,1),
(14286,1),
(14287,1),
(14288,1),
(14289,1),
(14290,1),
(14443,1),
(14517,1),
(14518,1),
(14873,1),
(14874,1),
(14895,1),
(14902,1),
(14903,1),
(14914,1),
(15040,1),
(15043,1),
(15095,1),
(15117,1),
(15124,1),
(15207,1),
(15208,1),
(15228,1),
(15230,1),
(15232,1),
(15234,1),
(15238,1),
(15242,1),
(15243,1),
(15244,1),
(15249,1),
(15254,1),
(15261,1),
(15262,1),
(15263,1),
(15264,1),
(15265,1),
(15266,1),
(15267,1),
(15284,1),
(15285,1),
(15305,1),
(15407,1),
(15451,1),
(15472,1),
(15496,1),
(15497,1),
(15498,1),
(15530,1),
(15536,1),
(15537,1),
(15547,1),
(15549,1),
(15574,1),
(15575,1),
(15581,1),
(15583,1),
(15598,1),
(15607,1),
(15611,1),
(15612,1),
(15615,1),
(15619,1),
(15620,1),
(15653,1),
(15659,1),
(15662,1),
(15665,1),
(15667,1),
(15668,1),
(15691,1),
(15692,1),
(15735,1),
(15736,1),
(15790,1),
(15791,1),
(15795,1),
(15797,1),
(15801,1),
(15860,1),
(15968,1),
(16000,1),
(16001,1),
(16006,1),
(16033,1),
(16044,1),
(16067,1),
(16100,1),
(16101,1),
(16144,1),
(16243,1),
(16249,1),
(16250,1),
(16375,1),
(16407,1),
(16408,1),
(16409,1),
(16410,1),
(16412,1),
(16413,1),
(16414,1),
(16415,1),
(16430,1),
(16454,1),
(16496,1),
(16511,1),
(16564,1),
(16565,1),
(16570,1),
(16603,1),
(16614,1),
(16697,1),
(16784,1),
(16788,1),
(16790,1),
(16799,1),
(16827,1),
(16828,1),
(16829,1),
(16830,1),
(16831,1),
(16832,1),
(16868,1),
(16908,1),
(16979,1),
(16997,1),
(17008,1),
(17140,1),
(17141,1),
(17142,1),
(17143,1),
(17144,1),
(17147,1),
(17149,1),
(17157,1),
(17171,1),
(17173,1),
(17174,1),
(17198,1),
(17203,1),
(17238,1),
(17243,1),
(17253,1),
(17255,1),
(17256,1),
(17257,1),
(17258,1),
(17259,1),
(17260,1),
(17261,1),
(17273,1),
(17274,1),
(17276,1),
(17281,1),
(17284,1),
(17290,1),
(17311,1),
(17312,1),
(17313,1),
(17314,1),
(17333,1),
(17347,1),
(17348,1),
(17353,1),
(17364,1),
(17393,1),
(17434,1),
(17435,1),
(17483,1),
(17501,1),
(17503,1),
(17509,1),
(17620,1),
(17682,1),
(17745,1),
(17877,1),
(17919,1),
(17920,1),
(17921,1),
(17922,1),
(17923,1),
(17924,1),
(17925,1),
(17926,1),
(17962,1),
(17963,1),
(18081,1),
(18082,1),
(18083,1),
(18084,1),
(18085,1),
(18089,1),
(18091,1),
(18092,1),
(18104,1),
(18105,1),
(18108,1),
(18111,1),
(18138,1),
(18164,1),
(18165,1),
(18187,1),
(18199,1),
(18204,1),
(18205,1),
(18211,1),
(18214,1),
(18217,1),
(18276,1),
(18278,1),
(18392,1),
(18398,1),
(18545,1),
(18557,1),
(18561,1),
(18649,1),
(18651,1),
(18796,1),
(18807,1),
(18809,1),
(18817,1),
(18819,1),
(18833,1),
(18867,1),
(18868,1),
(18869,1),
(18870,1),
(18871,1),
(18980,1),
(18984,1),
(19128,1),
(19260,1),
(19306,1),
(19319,1),
(19386,1),
(19391,1),
(19434,1),
(19463,1),
(19472,1),
(19503,1),
(19632,1),
(19639,1),
(19642,1),
(19701,1),
(19715,1),
(19725,1),
(19728,1),
(19729,1),
(19767,1),
(19777,1),
(19781,1),
(19785,1),
(19801,1),
(19816,1),
(19874,1),
(19881,1),
(19983,1),
(20228,1),
(20240,1),
(20243,1),
(20252,1),
(20276,1),
(20295,1),
(20297,1),
(20298,1),
(20420,1),
(20463,1),
(20535,1),
(20536,1),
(20543,1),
(20559,1),
(20560,1),
(20569,1),
(20605,1),
(20616,1),
(20617,1),
(20627,1),
(20630,1),
(20657,1),
(20658,1),
(20660,1),
(20661,1),
(20662,1),
(20666,1),
(20678,1),
(20684,1),
(20691,1),
(20692,1),
(20695,1),
(20696,1),
(20698,1),
(20714,1),
(20720,1),
(20735,1),
(20736,1),
(20743,1),
(20791,1),
(20792,1),
(20793,1),
(20795,1),
(20797,1),
(20802,1),
(20805,1),
(20806,1),
(20807,1),
(20808,1),
(20811,1),
(20815,1),
(20816,1),
(20819,1),
(20820,1),
(20821,1),
(20822,1),
(20823,1),
(20824,1),
(20825,1),
(20828,1),
(20829,1),
(20831,1),
(20832,1),
(20869,1),
(20883,1),
(20900,1),
(20901,1),
(20902,1),
(20903,1),
(20904,1),
(20909,1),
(20910,1),
(21008,1),
(21027,1),
(21047,1),
(21059,1),
(21071,1),
(21072,1),
(21077,1),
(21141,1),
(21151,1),
(21159,1),
(21162,1),
(21167,1),
(21170,1),
(21343,1),
(21354,1),
(21369,1),
(21390,1),
(21402,1),
(21549,1),
(21551,1),
(21552,1),
(21553,1),
(21667,1),
(21668,1),
(21807,1),
(21832,1),
(21978,1),
(22088,1),
(22121,1),
(22272,1),
(22273,1),
(22336,1),
(22355,1),
(22357,1),
(22411,1),
(22414,1),
(22425,1),
(22427,1),
(22568,1),
(22570,1),
(22574,1),
(22643,1),
(22666,1),
(22677,1),
(22746,1),
(22827,1),
(22828,1),
(22829,1),
(22859,1),
(22878,1),
(22887,1),
(22907,1),
(22908,1),
(23073,1),
(23102,1),
(23106,1),
(23206,1),
(23267,1),
(23391,1),
(23392,1),
(23512,1),
(23592,1),
(23601,1),
(23848,1),
(23850,1),
(23860,1),
(23881,1),
(23892,1),
(23893,1),
(23894,1),
(23922,1),
(23923,1),
(23924,1),
(23925,1),
(23959,1),
(23960,1),
(24042,1),
(24049,1),
(24132,1),
(24133,1),
(24238,1),
(24239,1),
(24248,1),
(24274,1),
(24275,1),
(24300,1),
(24331,1),
(24332,1),
(24334,1),
(24335,1),
(24374,1),
(24393,1),
(24407,1),
(24583,1),
(24585,1),
(24586,1),
(24587,1),
(24619,1),
(24640,1),
(24668,1),
(24680,1),
(24844,1),
(24942,1),
(25008,1),
(25009,1),
(25010,1),
(25011,1),
(25012,1),
(25021,1),
(25052,1),
(25054,1),
(25055,1),
(25174,1),
(25286,1),
(25288,1),
(25294,1),
(25295,1),
(25298,1),
(25300,1),
(25302,1),
(25304,1),
(25306,1),
(25307,1),
(25309,1),
(25345,1),
(25346,1),
(25677,1),
(25686,1),
(25710,1),
(25712,1),
(25783,15),
(25902,1),
(25911,1),
(25912,1),
(25995,1),
(26006,1),
(26098,1),
(26282,1),
(26350,1),
(26616,1),
(26693,1),
(27360,1),
(27552,1),
(27554,1),
(27559,1),
(27567,1),
(27572,1),
(27580,1),
(27581,1),
(27584,1),
(27611,1),
(27615,1),
(27632,1),
(27633,1),
(27794,1),
(27860,1),
(28318,1),
(28412,1),
(28478,1),
(28599,1),
(29070,1),
(29155,1),
(29228,1),
(29317,1),
(29515,1),
(30095,1),
(31016,1),
(31018,1);
/*!40000 ALTER TABLE `spell_facing` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_learn_spell`
--

DROP TABLE IF EXISTS `spell_learn_spell`;
CREATE TABLE `spell_learn_spell` (
  `entry` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SpellID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `Active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`entry`,`SpellID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item System';

--
-- Dumping data for table `spell_learn_spell`
--

LOCK TABLES `spell_learn_spell` WRITE;
/*!40000 ALTER TABLE `spell_learn_spell` DISABLE KEYS */;
INSERT INTO `spell_learn_spell` VALUES
(2842,8681,1),
(5784,33388,1),
(13819,33388,1),
(17002,24867,0),
(23161,33391,1),
(23214,33391,1),
(24866,24864,0);
/*!40000 ALTER TABLE `spell_learn_spell` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_pet_auras`
--

DROP TABLE IF EXISTS `spell_pet_auras`;
CREATE TABLE `spell_pet_auras` (
  `spell` mediumint(8) unsigned NOT NULL COMMENT 'dummy spell id',
  `pet` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'pet id; 0 = all',
  `aura` mediumint(8) unsigned NOT NULL COMMENT 'pet aura id',
  PRIMARY KEY (`spell`,`pet`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_pet_auras`
--

LOCK TABLES `spell_pet_auras` WRITE;
/*!40000 ALTER TABLE `spell_pet_auras` DISABLE KEYS */;
INSERT INTO `spell_pet_auras` VALUES
(19028,0,25228),
(19578,0,19579),
(20895,0,24529),
(28757,0,28758),
(23785,416,23759),
(23822,416,23826),
(23823,416,23827),
(23824,416,23828),
(23825,416,23829),
(23785,417,23762),
(23822,417,23837),
(23823,417,23838),
(23824,417,23839),
(23825,417,23840),
(23785,1860,23760),
(23822,1860,23841),
(23823,1860,23842),
(23824,1860,23843),
(23825,1860,23844),
(23785,1863,23761),
(23822,1863,23833),
(23823,1863,23834),
(23824,1863,23835),
(23825,1863,23836);
/*!40000 ALTER TABLE `spell_pet_auras` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_proc_event`
--

DROP TABLE IF EXISTS `spell_proc_event`;
CREATE TABLE `spell_proc_event` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SchoolMask` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyName` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask0` bigint(40) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask1` bigint(40) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask2` bigint(40) unsigned NOT NULL DEFAULT '0',
  `procFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `procEx` int(10) unsigned NOT NULL DEFAULT '0',
  `ppmRate` float NOT NULL DEFAULT '0',
  `CustomChance` float NOT NULL DEFAULT '0',
  `Cooldown` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_proc_event`
--

LOCK TABLES `spell_proc_event` WRITE;
/*!40000 ALTER TABLE `spell_proc_event` DISABLE KEYS */;
INSERT INTO `spell_proc_event` VALUES
(  324, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  3),
( 2565, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
( 6346, 0x7F,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000100, 0.000000, 0.000000,  0),
( 6866, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000070, 0.000000, 0.000000,  0),
( 7131, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000008, 0x00000020, 0.000000, 0.000000,  0),
( 9452, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
( 9782, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
( 9784, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
( 9799, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(11103, 0x04,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(11119, 0x04,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(11129, 0x04,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(11180, 0x10,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(11185, 0x00,  3, 0x0000000000000080, 0x0000000000000080, 0x0000000000000080, 0x00050000, 0x00000000, 0.000000, 0.000000,  0),
(11255, 0x00,  3, 0x0000000000004000, 0x0000000000004000, 0x0000000000004000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(12099, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(12169, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(12284, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.332520, 0.000000,  3),
(12289, 0x00,  0, 0x0000000000000002, 0x0000000000000002, 0x0000000000000002, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(12298, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(12311, 0x00,  4, 0x0000000000000800, 0x0000000000000800, 0x0000000000000800, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(12317, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(12319, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(12322, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 2.000000, 0.000000,  3),
(12701, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.665040, 0.000000,  3),
(12702, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.997560, 0.000000,  3),
(12703, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 1.330080, 0.000000,  3),
(12704, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 1.662600, 0.000000,  3),
(12782, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(12797, 0x00,  0, 0x0000000000000400, 0x0000000000000400, 0x0000000000000400, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(12834, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(12880, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00010000, 0.000000, 0.000000,  0),
(12966, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00010000, 0.000000, 0.000000,  0),
(12999, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 4.000000, 0.000000,  3),
(13000, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 6.000000, 0.000000,  3),
(13001, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 8.000000, 0.000000,  3),
(13002, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 10.00000, 0.000000,  3),
(13754, 0x00,  0, 0x0000000000000010, 0x0000000000000010, 0x0000000000000010, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(13877, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000074, 0.000000, 0.000000,  0),
(13983, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000018, 0.000000, 0.000000,  0),
(14156, 0x00,  0, 0x00000000003E0000, 0x00000000003E0000, 0x00000000003E0000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(14186, 0x00,  8, 0x0000000040800508, 0x0000000040800508, 0x0000000040800508, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(14531, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(14892, 0x00,  6, 0x0000000410001E00, 0x0000000410001E00, 0x0000000410001E00, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(15088, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(15268, 0x20,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(15277, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 6.000000, 0.000000,  3),
(15286, 0x20,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(15346, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 6.000000, 0.000000,  3),
(15600, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.600000, 0.000000,  3),
(16164, 0x1C,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(16176, 0x00, 11, 0x00000000000001C0, 0x00000000000001C0, 0x00000000000001C0, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(16256, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(16257, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00010000, 0.000000, 0.000000,  0),
(16487, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(16620, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000, 30),
(16624, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(16850, 0x00,  0, 0x0000000000000004, 0x0000000000000004, 0x0000000000000004, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(16864, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 2.000000, 0.000000,  3),
(16880, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(16952, 0x00,  0, 0x0000040000039000, 0x0000040000039000, 0x0000040000039000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(16958, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(17079, 0x00,  7, 0x0000000000080000, 0x0000000000080000, 0x0000000000080000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(17364, 0x08,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(17495, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(17670, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000008, 0x00000000, 0.000000, 0.000000,  0),
(17687, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000074, 0.000000, 0.000000,  0),
(17793, 0x00,  5, 0x0000000000000001, 0x0000000000000001, 0x0000000000000001, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(18094, 0x00,  5, 0x000000000000000A, 0x000000000000000A, 0x000000000000000A, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(18096, 0x00,  0, 0x0000008000000060, 0x0000008000000060, 0x0000008000000060, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(18119, 0x00,  5, 0x00000000000023e5, 0x00000000000023e5, 0x00000000000023e5, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(18137, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  3),
(18765, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000074, 0.000000, 0.000000,  0),
(18800, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000008, 0x00000000, 0.000000, 0.000000,  0),
(19228, 0x00,  0, 0x0000000000000040, 0x0000000000000040, 0x0000000000000040, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(19407, 0x00,  0, 0x0000000000000200, 0x0000000000000200, 0x0000000000000200, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(19572, 0x00,  9, 0x0000000000800000, 0x0000000000800000, 0x0000000000800000, 0x00004000, 0x00000000, 0.000000, 0.000000,  0),
(20049, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20127, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20128, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20131, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20132, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20133, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20134, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20164, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 5.000000, 0.000000,  3),
(20165, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 20.00000, 0.000000,  3),
(20166, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 20.00000, 0.000000,  3),
(20177, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20210, 0x00, 10, 0x00000000C0200000, 0x00000000C0200000, 0x00000000C0200000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20234, 0x00,  0, 0x0000000000008000, 0x0000000000008000, 0x0000000000008000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(20375, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 7.000000, 0.000000,  3),
(20500, 0x00,  4, 0x0000000010000000, 0x0000000010000000, 0x0000000010000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(20725, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20784, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20891, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(20911, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(20925, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(21185, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000, 10),
(21882, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(21890, 0x00,  4, 0x0000036C2A764EEF, 0x0000036C2A764EEF, 0x0000036C2A764EEF, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(22007, 0x00,  0, 0x0000000000200821, 0x0000000000200821, 0x0000000000200821, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(22618, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(22620, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(22648, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(23547, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000020, 0.000000, 0.000000,  0),
(23548, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(23551, 0x00,  0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(23552, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  3),
(23572, 0x00,  0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(23578, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 2.000000, 0.000000,  3),
(23581, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 2.000000, 0.000000,  3),
(23582, 0x00,  8, 0x0000000000000800, 0x0000000000000800, 0x0000000000000800, 0x00004000, 0x00000000, 0.000000, 0.000000,  0),
(23686, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 2.000000, 0.000000,  3),
(23689, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 4.000000, 0.000000,  3),
(23721, 0x00,  0, 0x0000000000000800, 0x0000000000000800, 0x0000000000000800, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(24658, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00014110, 0x00000000, 0.000000, 0.000000,  0),
(25669, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 1.000000, 0.000000,  3),
(25899, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000040, 0.000000, 0.000000,  0),
(26107, 0x00,  7, 0x0000008000800000, 0x0000008000800000, 0x0000008000800000, 0x00000000, 0x00000074, 0.000000, 0.000000,  0),
(26119, 0x00, 11, 0x0000000090100003, 0x0000000090100003, 0x0000000090100003, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(26128, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000008, 0.000000, 0.000000,  0),
(26135, 0x00,  0, 0x0000000000800000, 0x0000000000800000, 0x0000000000800000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(26480, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(27419, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(27498, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(27656, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(27787, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(27811, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(28716, 0x00,  7, 0x0000000000000010, 0x0000000000000010, 0x0000000000000010, 0x00048000, 0x00000000, 0.000000, 0.000000,  0),
(28719, 0x00,  7, 0x0000000000000020, 0x0000000000000020, 0x0000000000000020, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(28744, 0x00,  7, 0x0000000000000040, 0x0000000000000040, 0x0000000000000040, 0x00044000, 0x00000000, 0.000000, 0.000000,  0),
(28752, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(28789, 0x00, 10, 0x0000000000006000, 0x0000000000006000, 0x0000000000006000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(28809, 0x00,  0, 0x0000000000001000, 0x0000000000001000, 0x0000000000001000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(28812, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(28816, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(28823, 0x00,  0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000000000C0, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(28847, 0x00,  7, 0x0000000000000020, 0x0000000000000020, 0x0000000000000020, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(28849, 0x00, 11, 0x0000000000000080, 0x0000000000000080, 0x0000000000000080, 0x00000000, 0x00000000, 0.000000, 0.000000,  0),
(29062, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(29074, 0x14,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(29150, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29441, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000008, 0.000000, 0.000000,  1),
(29501, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29624, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29625, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29626, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29632, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29633, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29634, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29635, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29636, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(29637, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000, 3.000000, 0.000000,  0),
(30160, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(30003, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000800, 0.000000, 0.000000,  0),
(30802, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(30808, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(30809, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(30810, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0),
(30811, 0x00,  0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000002, 0.000000, 0.000000,  0);
/*!40000 ALTER TABLE `spell_proc_event` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_proc_item_enchant`
--

DROP TABLE IF EXISTS `spell_proc_item_enchant`;
CREATE TABLE `spell_proc_item_enchant` (
  `entry` mediumint(8) unsigned NOT NULL,
  `ppmRate` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `spell_proc_item_enchant`
--

LOCK TABLES `spell_proc_item_enchant` WRITE;
/*!40000 ALTER TABLE `spell_proc_item_enchant` DISABLE KEYS */;
INSERT INTO `spell_proc_item_enchant` VALUES
(8034, 9),        -- Frostbrand Weapon
(13897, 6.0),     -- Enchant Weapon - Fiery Weapon
(20004, 6.0),     -- Enchant Weapon - Lifestealing
(20005, 1.6);     -- Enchant Weapon - Icy Chill
/*!40000 ALTER TABLE `spell_proc_item_enchant` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_script_target`
--

DROP TABLE IF EXISTS `spell_script_target`;
CREATE TABLE `spell_script_target` (
  `entry` mediumint(8) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `targetEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `inverseEffectMask` mediumint(8) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `entry_type_target` (`entry`,`type`,`targetEntry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Spell System';

--
-- Dumping data for table `spell_script_target`
--

LOCK TABLES `spell_script_target` WRITE;
/*!40000 ALTER TABLE `spell_script_target` DISABLE KEYS */;
/*!40000 ALTER TABLE `spell_script_target` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_target_position`
--

DROP TABLE IF EXISTS `spell_target_position`;
CREATE TABLE `spell_target_position` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `target_map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `target_position_x` float NOT NULL DEFAULT '0',
  `target_position_y` float NOT NULL DEFAULT '0',
  `target_position_z` float NOT NULL DEFAULT '0',
  `target_orientation` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Spell System';

--
-- Dumping data for table `spell_target_position`
--

LOCK TABLES `spell_target_position` WRITE;
/*!40000 ALTER TABLE `spell_target_position` DISABLE KEYS */;
INSERT INTO `spell_target_position` VALUES
(442,129,2592.55,1107.5,51.29,4.74),
(444,1,-4658.12,-2526.35,82.9671,0),
(446,109,-319.24,99.9,-131.85,3.19),
(3561,0,-9003.46,870.031,29.6206,5.28),
(3562,0,-4613.62,-915.38,501.062,3.88),
(3563,0,1773.47,61.121,-46.3207,0.54),
(3565,1,9660.81,2513.64,1331.66,3.06),
(3566,1,-964.98,283.433,111.187,3.02),
(3567,1,1469.85,-4221.52,58.9939,5.98),
(17334,0,-9003.46,870.031,29.6206,2.28),
(17608,1,9660.81,2513.64,1331.66,3.06),
(17609,1,1469.85,-4221.52,58.9939,5.98),
(17610,1,-964.98,283.433,111.187,3.02),
(17611,0,1773.47,61.121,-46.3207,0.54),
(18960,1,7991.88,-2679.84,512.1,0.034183),
(23442,1,6755.33,-4658.09,724.8,3.4049),
(12510,1,3778.74,-4611.78,227.252,4.23263),
(29273,533,3498.28,-5349.9,144.968,1.31324),
(33,0,-10643,1052,34,0),
(31,0,-9464,62,56,0),
(34,0,-10368,-422,66,0),
(35,0,-9104,-70,83,0),
(427,189,1688.99,1053.48,18.6775,0.00117),
(428,0,-11020,1436,44,0),
(445,0,-10566,-1189,28,0),
(4996,1,1552.5,-4420.66,8.94802,0),
(4997,0,-14457,496.45,39.1392,0),
(4998,0,-12415,207.618,31.5017,0.124875),
(4999,1,-998.359,-3827.52,5.44507,4.16654),
(6348,0,-3752.81,-851.558,10.1153,0),
(6349,1,6581.05,767.5,5.78428,6.01616),
(6483,1,5483.9,-749.881,334.621,0),
(6719,1,-3615.49,-4467.34,24.3141,0),
(8735,48,-151.89,106.96,-39.87,4.53),
(9268,1,8786.36,967.445,30.197,3.39632),
(11362,90,-332.22,-2.28,-150.86,2.77),
(11409,0,-13184,334,100,0),
(447,0,16229,16265,14,3.19),
(3721,0,16229,16265,14,3.19),
(1936,0,16229,16265,14,0),
(443,0,16229,16265,14,4.74),
(8195,0,16229,16265,14,3.19),
(12520,1,3644.95,-4700.25,120.81,6.25),
(12241,1,-3282.15,2062.17,245.96,4.15),
(6766,1,-2354.03,-1902.07,95.78,4.6),
(12885,0,-11238.4,-2831.97,157.93,5.11),
(13142,0,-11238.4,-2831.97,157.93,5.11),
(25725,509,-9717.2,1517.81,27.6,5.4),
(6714,1,-4884.49,-1596.2,101.2,3.17),
(23441,1,-7109.1,-3825.21,10.151,2.8331),
(21128,349,419.84,11.3365,-131.079,0),
(17159,1,6106.4,-4191.1,849.74,0.0201),
(17160,1,5904.2,-4045.9,596.43,0.0083),
(23446,1,-7109.1,-3825.21,10.151,2.8331),
(22651,429,-25.98,-448.24,-36.09,3.13),
(8995,0,-2646.62,-2455.57,80.87,0),
(8996,0,-2645.25,-2449.5,82.65,0),
(8997,0,-2648.88,-2442.54,80.71,0),
(9055,0,-1350.83,-2740.16,62.74,2.95),
(12158,90,-531.09,-103.04,-156.04,0),
(12159,90,-510.59,-102.89,-152.46,0),
(13912,230,1380.58,-819.48,-92.72,4.69),
(23460,469,-7672.46,-1107.19,396.65,0.59),
(24325,309,-11726.3,-1777.38,10.46,0.16),
(24593,309,-11726.3,-1777.38,10.46,0.16),
(8606,0,325.8,-1481.47,42.68,0.47),
(11012,1,-9619.32,-2815.02,10.89,0.37),
(13044,0,-11224.8,-2835.35,158.85,3.73),
(13461,0,-11234.8,-2842.52,157.92,1.47),
(22950,429,-39.39,812.47,-11.64,4.77),
(22951,1,-3760.87,1086.28,131.96,1.53),
(22191,249,2.96,-198.83,-86.57,0),
(22192,249,-4.92,-248.93,-86.84,0),
(22193,249,-12.64,-183.21,-87.35,0),
(22194,249,13.56,-234.31,-85.56,0),
(22195,249,14.08,-219.35,-85.87,0),
(22196,249,18.79,-194.5,-85.01,0),
(22197,249,2.43,-185.71,-86.65,0),
(22198,249,-16.71,-196.87,-88.16,0),
(22199,249,-16.14,-214.12,-88.46,0),
(22200,249,-54.89,-229.6,-85.48,0),
(22201,249,-40.45,-211.84,-86.66,0),
(22202,249,-16.69,-232.6,-88.13,0),
(17475,329,4017.4,-3339.7,115.058,5.48),
(17476,329,4013.18,-3351.8,115.052,0.13),
(17477,329,4017.73,-3363.47,115.057,0.72),
(17478,329,4048.87,-3363.22,115.054,3.62),
(17479,329,4051.77,-3350.89,115.055,3.06),
(17480,329,4048.37,-3339.96,115.055,2.45),
(17607,0,-4613.62,-915.38,501.062,3.88);
/*!40000 ALTER TABLE `spell_target_position` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spell_threat`
--

DROP TABLE IF EXISTS `spell_threat`;
CREATE TABLE `spell_threat` (
  `entry` mediumint(8) unsigned NOT NULL,
  `Threat` smallint(6) NOT NULL,
  `multiplier` float NOT NULL DEFAULT '1' COMMENT 'threat multiplier for damage/healing',
  `ap_bonus` float NOT NULL DEFAULT '0' COMMENT 'additional threat bonus from attack power',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `spell_threat`
--

LOCK TABLES `spell_threat` WRITE;
/*!40000 ALTER TABLE `spell_threat` DISABLE KEYS */;
INSERT INTO `spell_threat` VALUES
(   72, 180, 1, 0),
(   78,  20, 1, 0),
(   99,  15, 1, 0),
(  284,  39, 1, 0),
(  285,  59, 1, 0),
(  770, 108, 1, 0),
(  845,  10, 1, 0),
( 1608,  78, 1, 0),
( 1715,  61, 1, 0),
( 1735,  25, 1, 0),
( 2139, 300, 1, 0),
( 2649, 415, 1, 0),
( 3716, 395, 1, 0),
( 6343,  17, 1, 0),
( 6572, 155, 1, 0),
( 6574, 195, 1, 0),
( 6673,  60, 1, 0),
( 6807,  59, 1, 0),
( 6809,  89, 1, 0),
( 7369,  40, 1, 0),
( 7372, 101, 1, 0),
( 7373, 141, 1, 0),
( 7379, 235, 1, 0),
( 7386, 100, 1, 0),
( 7405, 140, 1, 0),
( 8198,  40, 1, 0),
( 8204,  64, 1, 0),
( 8205,  96, 1, 0),
( 8380, 180, 1, 0),
( 8972, 118, 1, 0),
( 9490,  29, 1, 0),
( 9745, 148, 1, 0),
( 9747,  36, 1, 0),
( 9880, 178, 1, 0),
( 9881, 207, 1, 0),
( 9898,  42, 1, 0),
(11564,  98, 1, 0),
(11565, 118, 1, 0),
(11566, 137, 1, 0),
(11567, 145, 1, 0),
(11580, 143, 1, 0),
(11581, 180, 1, 0),
(11596, 220, 1, 0),
(11597, 261, 1, 0),
(11600, 275, 1, 0),
(11601, 315, 1, 0),
(11608,  60, 1, 0),
(11609,  70, 1, 0),
(14274, 200, 1, 0),
(15629, 300, 1, 0),
(15630, 400, 1, 0),
(15631, 500, 1, 0),
(15632, 600, 1, 0),
(16857, 108, 1, 0),
(17735, 200, 1, 0),
(17750, 300, 1, 0),
(17751, 450, 1, 0),
(17752, 600, 1, 0),
(20243, 101, 1, 0),
(20569, 100, 1, 0),
(20736, 100, 1, 0),
(20925,  20, 1, 0),
(20927,  30, 1, 0),
(20928,  40, 1, 0),
(23922, 160, 1, 0),
(23923, 190, 1, 0),
(23924, 220, 1, 0),
(23925, 250, 1, 0),
(24394, 580, 1, 0),
(24640,   5, 1, 0),
(25286, 175, 1, 0),
(25288, 355, 1, 0);
/*!40000 ALTER TABLE `spell_threat` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `taxi_shortcuts`
--

DROP TABLE IF EXISTS `taxi_shortcuts`;
CREATE TABLE `taxi_shortcuts` (
  `pathid` int unsigned NOT NULL COMMENT 'Flight path entry id',
  `takeoff` int unsigned NOT NULL COMMENT 'Amount of waypoints to skip in the beginning of the flight',
  `landing` int unsigned NOT NULL COMMENT 'Amount of waypoints to skip at the end of the flight',
  `comments` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`pathid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Taxi System';

--
-- Dumping data for table `taxi_shortcuts`
--

LOCK TABLES `taxi_shortcuts` WRITE;
/*!40000 ALTER TABLE `taxi_shortcuts` DISABLE KEYS */;
/*!40000 ALTER TABLE `taxi_shortcuts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `trainer_greeeting`
--

DROP TABLE IF EXISTS `trainer_greeting`;
CREATE TABLE `trainer_greeting` (
   `Entry` INT(11) UNSIGNED NOT NULL COMMENT 'Entry of Trainer',
   `Text` LONGTEXT COMMENT 'Text of the greeting',
   PRIMARY KEY(`Entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Trainer system';

--
-- Table structure for table `transports`
--

DROP TABLE IF EXISTS `transports`;
CREATE TABLE `transports` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name` text,
  `period` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Transports';

--
-- Dumping data for table `transports`
--

LOCK TABLES `transports` WRITE;
/*!40000 ALTER TABLE `transports` DISABLE KEYS */;
/*!40000 ALTER TABLE `transports` ENABLE KEYS */;
UNLOCK TABLES;

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
(89,1,'tamia.dll',NULL,0,0,'0',14,'Tamia hack'),
(90,0,NULL,NULL,12900744,4,'0000C843',2,'Nameplate extender'),
(91,0,NULL,NULL,8784512,4,'00006144',2,'Unlimited follow distance');

--
-- Table structure for table `waypoint_path`
--

DROP TABLE IF EXISTS `waypoint_path`;
CREATE TABLE `waypoint_path`  (
  `PathId` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Unique path id',
  `Point` mediumint(8) UNSIGNED NOT NULL DEFAULT 0,
  `PositionX` float NOT NULL DEFAULT 0,
  `PositionY` float NOT NULL DEFAULT 0,
  `PositionZ` float NOT NULL DEFAULT 0,
  `Orientation` float NOT NULL DEFAULT 0,
  `WaitTime` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `ScriptId` mediumint(8) UNSIGNED NOT NULL DEFAULT 0,
  `Comment` text NULL DEFAULT NULL,
  PRIMARY KEY (`PathId`, `Point`)
);

DROP TABLE IF EXISTS waypoint_path_name;
CREATE TABLE waypoint_path_name(
  `PathId` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Path ID for entry',
  `Name` VARCHAR(300) NOT NULL COMMENT 'Description of usage',
  PRIMARY KEY(`PathId`)
);

--
-- Table structure for table `world_safe_locs`
--

DROP TABLE IF EXISTS `world_safe_locs`;
CREATE TABLE `world_safe_locs` (
   `id` int(11) unsigned NOT NULL,
   `map` int(10) unsigned NOT NULL DEFAULT '0',
   `x` float NOT NULL DEFAULT '0',
   `y` float NOT NULL DEFAULT '0',
   `z` float NOT NULL DEFAULT '0',
   `o` float NOT NULL DEFAULT '0',
   `name` varchar(50) NOT NULL DEFAULT '',
   PRIMARY KEY (`id`)
 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `worldstate_name`
--

CREATE TABLE worldstate_name (
`Id` INT(11) NOT NULL COMMENT 'Worldstate variable Id',
`Name` VARCHAR(200) NOT NULL COMMENT 'Name and use of variable',
PRIMARY KEY(`Id`)
);

--
-- Table structure for table `world_template`
--

DROP TABLE IF EXISTS `world_template`;
CREATE TABLE `world_template` (
  `map` smallint(5) unsigned NOT NULL,
  `ScriptName` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`map`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `world_template`
--

LOCK TABLES `world_template` WRITE;
/*!40000 ALTER TABLE `world_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `world_template` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-09-10  0:00:00
