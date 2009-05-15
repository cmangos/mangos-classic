ALTER TABLE character_db_version CHANGE COLUMN required_032_7059_02_characters_pet_spell required_2008_12_15_01_character_arenas bit;

ALTER TABLE pet_spell
    ADD COLUMN `slot` int(11) unsigned NOT NULL default '0' AFTER `spell`;

ALTER TABLE character_spell
    ADD COLUMN `slot` int(11) unsigned NOT NULL default '0' AFTER `spell`;
