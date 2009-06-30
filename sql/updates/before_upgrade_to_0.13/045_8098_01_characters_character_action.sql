ALTER TABLE character_db_version CHANGE COLUMN required_045_8098_01_characters_character_action required_041_7903_01_characters_character_pet bit;

ALTER TABLE character_action
  ADD COLUMN misc int(11) unsigned NOT NULL default '0';
