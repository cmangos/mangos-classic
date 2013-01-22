ALTER TABLE db_version CHANGE COLUMN required_z0124_xxx_02_mangos required_z0131_xxx_01_mangos_creature_template  bit;


ALTER TABLE creature_template ADD COLUMN Civilian TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER unk17;
ALTER TABLE creature_template DROP COLUMN unk16;
ALTER TABLE creature_template DROP COLUMN unk17;