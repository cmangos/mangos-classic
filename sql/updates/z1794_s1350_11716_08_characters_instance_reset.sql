ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_07_characters_auction required_z1794_s1350_11716_08_characters_auction bit;

ALTER TABLE `instance_reset`
  CHANGE COLUMN `resettime` `resettime` bigint(40) unsigned NOT NULL default '0';
