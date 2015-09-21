ALTER TABLE db_version CHANGE COLUMN required_z1540_s1036_01_mangos_creature_addon required_z1540_s1036_02_mangos_creature_template_addon bit;

ALTER TABLE creature_template_addon
  ADD COLUMN b2_0_sheath tinyint(3) unsigned NOT NULL DEFAULT '0' AFTER bytes1,
  ADD COLUMN b2_1_flags tinyint(3) unsigned NOT NULL DEFAULT '0' AFTER b2_0_sheath;

UPDATE creature_template_addon SET b2_0_sheath = bytes2 & 0x000000FF;
UPDATE creature_template_addon SET b2_1_flags = (bytes2 & 0x0000FF00) >> 8;

ALTER TABLE creature_template_addon
  DROP COLUMN bytes2;
