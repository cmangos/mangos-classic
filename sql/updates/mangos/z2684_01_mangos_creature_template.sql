ALTER TABLE db_version CHANGE COLUMN required_z2683_01_mangos_scriptdev2_tables required_z2684_01_mangos_creature_template bit;

ALTER TABLE creature_template ADD COLUMN SchoolImmuneMask INT(10) UNSIGNED DEFAULT 0 NOT NULL AFTER MechanicImmuneMask; 