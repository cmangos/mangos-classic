ALTER TABLE character_db_version CHANGE COLUMN required_z0114_xxx_01_character_kill required_z0114_xxx_02_characters bit;


ALTER TABLE characters CHANGE COLUMN honor_rating stored_honor_rating FLOAT NOT NULL DEFAULT '0';

ALTER TABLE characters ADD COLUMN stored_dishonorable_kills INT(11) NOT NULL DEFAULT '0' AFTER stored_honor_rating;

ALTER TABLE characters ADD COLUMN stored_honorable_kills FLOAT NOT NULL DEFAULT '0' AFTER stored_dishonorable_kills;