ALTER TABLE db_version CHANGE COLUMN required_z1040_s0418_01_mangos_creature_template required_z1040_s0418_02_mangos_creature_addon bit;

ALTER TABLE creature_addon
  DROP COLUMN bytes0;
