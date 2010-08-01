ALTER TABLE db_version CHANGE COLUMN required_z0764_s0070_01_mangos_reputation_spillover_template required_z0767_s0075_01_mangos_creature_model_info bit;

ALTER TABLE creature_model_info ADD COLUMN modelid_other_team mediumint(8) unsigned NOT NULL default '0' AFTER modelid_other_gender;
