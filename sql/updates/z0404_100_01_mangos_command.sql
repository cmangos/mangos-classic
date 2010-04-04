ALTER TABLE db_version CHANGE COLUMN required_z0395_xxx_01_mangos_mangos_string required_z0404_100_01_mangos_command bit;

DELETE FROM command WHERE name IN ('list talents');
INSERT INTO command VALUES
('list talents',3,'Syntax: .list talents\r\n\r\nShow list all really known (as learned spells) talent rank spells for selected player or self.');
