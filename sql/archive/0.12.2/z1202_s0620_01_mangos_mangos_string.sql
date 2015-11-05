ALTER TABLE db_version CHANGE COLUMN required_z1194_s0611_01_mangos_mangos_string required_z1202_s0620_01_mangos_mangos_string bit;

UPDATE mangos_string SET content_default='Scripting library not found or not accessible.' WHERE entry=1166;
