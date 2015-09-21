ALTER TABLE character_db_version CHANGE COLUMN required_z0082_xxx_01_characters_character_social required_z0114_xxx_01_character_kill bit;

ALTER TABLE character_kill CHANGE COLUMN creature_template victim INT(11) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Creature / Player Identifier';
ALTER TABLE character_kill ADD COLUMN victim_type TINYINT(3) UNSIGNED NOT NULL DEFAULT '4' AFTER guid;
