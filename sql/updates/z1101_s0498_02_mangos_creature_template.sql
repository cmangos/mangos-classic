ALTER TABLE db_version CHANGE COLUMN required_z1101_s0498_01_mangos_npc_trainer_template required_z1101_s0498_02_mangos_creature_template bit;

ALTER TABLE creature_template
  ADD COLUMN trainer_id mediumint(8) unsigned NOT NULL default '0' AFTER equipment_id;
