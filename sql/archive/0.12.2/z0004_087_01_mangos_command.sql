ALTER TABLE db_version CHANGE COLUMN required_z0002_083_02_mangos_command required_z0004_087_01_mangos_command bit;

DELETE FROM command where name IN ('server shutdown');

INSERT INTO `command` VALUES
('server shutdown',3,'Syntax: .server shutdown #delay [#exit_code]\r\n\r\nShut the server down after #delay seconds. Use #exit_code or 0 as program exit code.');
