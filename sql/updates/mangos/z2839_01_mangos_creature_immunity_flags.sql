ALTER TABLE db_version CHANGE COLUMN required_z2837_01_mangos_gobject_near_link required_z2839_01_mangos_creature_immunity_flags bit;

ALTER TABLE creature_template ADD COLUMN `ImmunityFlags` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `SchoolImmuneMask`;


