ALTER TABLE db_version CHANGE COLUMN required_z0770_s0080_03_mangos_scripted_event_id required_z0775_s0083_01_mangos_mangos_string bit;

UPDATE mangos_string SET content_default = 'Unit Flags: %u.\nDynamic Flags: %u.\nFaction Template: %u.' WHERE entry = 542;