ALTER TABLE db_version CHANGE COLUMN required_z2621_s2263_12823_04_mangos_creature required_z2674_sxxxx_xxxxx_01_mangos_creature_template bit;

ALTER TABLE `creature_template`
ADD COLUMN `ModelId3` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `ModelId2`,
ADD COLUMN `ModelId4` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER `ModelId3`;
