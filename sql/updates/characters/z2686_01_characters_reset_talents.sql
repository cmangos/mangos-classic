ALTER TABLE character_db_version CHANGE COLUMN required_z2682_01_characters_pvpstats required_z2686_01_characters_reset_talents bit;

UPDATE characters SET at_login = at_login | 0x4;
