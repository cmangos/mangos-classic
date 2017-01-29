ALTER TABLE character_db_version CHANGE COLUMN required_z2686_01_characters_reset_talents required_z2694_01_characters_instance bit;

ALTER TABLE instance ADD COLUMN `encountersMask` int(10) unsigned NOT NULL DEFAULT '0' AFTER resettime;