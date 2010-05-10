ALTER TABLE character_db_version CHANGE COLUMN required_z0464_112_01_characters_character_skills required_z0510_114_01_characters_saved_variables bit;

ALTER TABLE saved_variables ADD cleaning_flags int(11) unsigned NOT NULL default '0' AFTER NextMaintenanceDate;
UPDATE saved_variables SET cleaning_flags = 0xF;
