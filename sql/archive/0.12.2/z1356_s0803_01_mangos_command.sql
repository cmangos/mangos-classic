ALTER TABLE db_version CHANGE COLUMN required_z1347_s0794_01_mangos_spell_threat required_z1356_s0803_01_mangos_command bit;

DELETE FROM command WHERE name IN ('gm online','gm setview');

INSERT INTO command (name, security, help) VALUES
('gm setview',1,'Syntax: .gm setview\r\n\r\nSet farsight view on selected unit. Select yourself to set view back.');

