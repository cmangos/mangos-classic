ALTER TABLE character_db_version CHANGE COLUMN required_045_8098_03_characters_character_pet required_045_8098_04_characters_pet_spell bit;

UPDATE pet_spell
   SET active = ( active >> 8);
