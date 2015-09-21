ALTER TABLE character_db_version CHANGE COLUMN required_z1794_s1350_11716_09_characters_auction required_z2410_s2063_12562_01_characters_various_tables bit;

ALTER TABLE character_social DROP INDEX friend;
ALTER TABLE character_social DROP INDEX guid;
ALTER TABLE guild_member DROP INDEX guildid_key;
