ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_04_characters_auction required_z1794_s1350_11716_05_characters_auction bit;

ALTER TABLE `guild`
  CHANGE COLUMN `createdate` `createdate` bigint(20) unsigned NOT NULL default '0';
