ALTER TABLE character_db_version CHANGE COLUMN required_2008_12_15_01_character_arenas required_032_7059_01_characters_character_spell bit;

ALTER TABLE character_spell
    DROP slot;
