ALTER TABLE db_version CHANGE COLUMN required_z2762_01_mangos_gm_mountup_command required_z2763_01_mangos_ability_sets bit;

ALTER TABLE creature_template_spells ADD COLUMN `setId` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Id of set of spells' AFTER `entry`;
ALTER TABLE creature_template_spells DROP PRIMARY KEY, ADD PRIMARY KEY(entry, setId);

