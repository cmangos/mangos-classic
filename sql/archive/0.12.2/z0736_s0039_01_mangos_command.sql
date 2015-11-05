ALTER TABLE db_version CHANGE COLUMN required_z0735_s0038_01_mangos_command required_z0736_s0039_01_mangos_command bit;

DELETE FROM command WHERE name = 'stable';
INSERT INTO command (name, security, help) VALUES
('stable',3,'Syntax: .stable\r\n\r\nShow your pet stable.');
