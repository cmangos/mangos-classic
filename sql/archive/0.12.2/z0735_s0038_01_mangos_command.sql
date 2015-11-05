ALTER TABLE db_version CHANGE COLUMN required_z0726_s0028_01_mangos_playercreateinfo required_z0735_s0038_01_mangos_command bit;

DELETE FROM command WHERE name = 'wp';
