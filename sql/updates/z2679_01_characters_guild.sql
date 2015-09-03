ALTER TABLE character_db_version CHANGE COLUMN required_z2678_02_characters required_z2679_01_characters_guild bit;

ALTER TABLE `guild` CHANGE `info` `info` varchar(500) NOT NULL DEFAULT '';
ALTER TABLE `guild` CHANGE `motd` `motd` varchar(128) NOT NULL DEFAULT '';
