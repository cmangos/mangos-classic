ALTER TABLE character_db_version CHANGE COLUMN required_z1400_xxxxx_01_characters_characters required_z1401_s0860_01_characters_corpse bit;

ALTER TABLE corpse
  DROP COLUMN data,
  DROP COLUMN zone;
