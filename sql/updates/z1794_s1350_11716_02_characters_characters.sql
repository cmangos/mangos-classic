ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_01_characters_auction required_z1794_s1350_11716_02_characters_auction bit;

ALTER TABLE `characters`
  CHANGE COLUMN `deleteDate` `deleteDate` bigint(20) unsigned default NULL;
