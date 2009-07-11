ALTER TABLE character_db_version CHANGE COLUMN required_052_8072_01_characters_characters required_045_8098_04_characters_pet_spell bit;

ALTER TABLE characters
DROP COLUMN gender,
DROP COLUMN level,
DROP COLUMN xp,
DROP COLUMN money,
DROP COLUMN playerBytes,
DROP COLUMN playerBytes2,
DROP COLUMN playerFlags;
