ALTER TABLE character_db_version CHANGE COLUMN required_z2744_01_characters_WorldState required_z2745_01_characters_weekly_quests bit;

DROP TABLE IF EXISTS `character_queststatus_weekly`;
CREATE TABLE `character_queststatus_weekly` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `quest` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`guid`,`quest`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

ALTER TABLE saved_variables
ADD `NextWeeklyQuestResetTime` bigint(40) unsigned NOT NULL DEFAULT '0';

