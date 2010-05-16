ALTER TABLE db_version CHANGE COLUMN required_z0524_115_02_mangos_scripts required_z0525_116_01_mangos_creature_movement bit;

ALTER TABLE creature_movement ADD COLUMN script_id MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER waittime;
