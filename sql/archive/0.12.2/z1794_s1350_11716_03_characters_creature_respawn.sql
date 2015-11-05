ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_02_characters_auction required_z1794_s1350_11716_03_characters_auction bit;

ALTER TABLE `creature_respawn`
  CHANGE COLUMN `respawntime` `respawntime` bigint(20) unsigned NOT NULL default '0';
