ALTER TABLE character_db_version CHANGE COLUMN required_z2786_01_characters_shaman_talent required_z2796_01_characters_fishingSteps bit;

ALTER TABLE characters ADD COLUMN `fishingSteps` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `grantableLevels`;


