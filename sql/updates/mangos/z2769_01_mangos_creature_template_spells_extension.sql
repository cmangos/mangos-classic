ALTER TABLE db_version CHANGE COLUMN required_z2767_01_mangos_power_regen required_z2769_01_mangos_creature_template_spells_extension bit;

ALTER TABLE creature_template_spells ADD COLUMN spell9 MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER spell8;
ALTER TABLE creature_template_spells ADD COLUMN spell10 MEDIUMINT UNSIGNED NOT NULL DEFAULT '0' AFTER spell9;
ALTER TABLE creature_template_spells DROP PRIMARY KEY, ADD PRIMARY KEY(entry, setId);

