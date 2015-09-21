ALTER TABLE character_db_version CHANGE COLUMN required_z1014_s0386_01_characters_item_loot required_z1126_s0507_01_characters_characters bit;

ALTER TABLE characters DROP COLUMN bgid;
ALTER TABLE characters DROP COLUMN bgteam;
ALTER TABLE characters DROP COLUMN bgmap;
ALTER TABLE characters DROP COLUMN bgx;
ALTER TABLE characters DROP COLUMN bgy;
ALTER TABLE characters DROP COLUMN bgz;
ALTER TABLE characters DROP COLUMN bgo;
