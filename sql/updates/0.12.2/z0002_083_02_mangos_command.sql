ALTER TABLE db_version CHANGE COLUMN required_z0002_083_01_mangos_mangos_string required_z0002_083_02_mangos_command bit;

DELETE FROM command where name IN ('quit');

INSERT INTO `command` VALUES
('quit',4,'Syntax: quit\r\n\r\nClose RA connection. Command must be typed fully (quit).');
