ALTER TABLE db_version CHANGE COLUMN required_z0004_087_01_mangos_command required_z0013_088_01_mangos_command bit;

DELETE FROM command where name IN ('modify scale');

INSERT INTO `command` VALUES
('modify scale',1,'Syntax: .modify scale #scale\r\n\r\nChange model scale for targeted player (util relogin) or creature (until respawn).');
