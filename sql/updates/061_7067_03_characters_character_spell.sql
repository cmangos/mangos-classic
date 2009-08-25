ALTER TABLE character_db_version CHANGE COLUMN required_052_8072_01_characters_characters required_061_7067_03_characters_character_spell bit;

DELETE FROM `character_spell` WHERE `spell` IN (7376,3025,5419,5421,21156,7381,1178,21178,9635,21178,24905,5420,34123,33948,34764,40121,40122);
