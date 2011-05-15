ALTER TABLE db_version CHANGE COLUMN required_z1428_s0902_01_mangos_mangos_string required_z1428_s0902_02_mangos_command bit;

DELETE FROM command WHERE name = 'debug spellcoefs';

INSERT INTO command (name, security, help) VALUES
('debug spellcoefs',3,'Syntax: .debug spellcoefs #spellid\r\n\r\nShow default calculated and DB stored coefficients for direct/dot heal/damage.');
