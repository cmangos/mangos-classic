ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_08_characters_auction required_z1794_s1350_11716_09_characters_auction bit;

ALTER TABLE `mail`
  CHANGE COLUMN `expire_time` `expire_time` bigint(40) unsigned NOT NULL default '0',
  CHANGE COLUMN `deliver_time` `deliver_time` bigint(40) unsigned NOT NULL default '0';

