ALTER TABLE character_db_version CHANGE COLUMN required_z1135_s0540_02_characters_character_spell required_z1142_s0531_01_characters_bugreport bit;

ALTER TABLE `bugreport` CHANGE `type` `type` LONGTEXT NOT NULL;
ALTER TABLE `bugreport` CHANGE `content` `content` LONGTEXT NOT NULL;
