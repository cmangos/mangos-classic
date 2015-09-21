ALTER TABLE db_version CHANGE COLUMN required_z0131_xxx_01_mangos_creature_template required_z0133_xxx_01_mangos_creature_template BIT;

ALTER TABLE creature_template CHANGE COLUMN modelid_A modelid_1 MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE creature_template CHANGE COLUMN modelid_A2 modelid_2 MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0';

ALTER TABLE creature_template DROP COLUMN modelid_H;
ALTER TABLE creature_template DROP COLUMN modelid_H2;