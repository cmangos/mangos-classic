ALTER TABLE db_version CHANGE COLUMN required_z1018_s0391_01_mangos_npc_vendor_template required_z1018_s0391_02_mangos_creature_template bit;

ALTER TABLE creature_template
  ADD COLUMN vendor_id mediumint(8) unsigned NOT NULL default '0' AFTER equipment_id;
