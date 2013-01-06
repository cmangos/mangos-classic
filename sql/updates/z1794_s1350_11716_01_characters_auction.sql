ALTER TABLE character_db_version CHANGE COLUMN required_z1787_s1342_11704_01_characters_auction required_z1794_s1350_11716_01_characters_auction bit;

ALTER TABLE `auction`
  CHANGE COLUMN `time` `time` bigint(40) unsigned NOT NULL default '0';
