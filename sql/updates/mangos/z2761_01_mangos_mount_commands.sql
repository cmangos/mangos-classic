ALTER TABLE db_version CHANGE COLUMN required_z2760_01_mangos_command required_z2761_01_mangos_mount_commands bit;

DELETE FROM command WHERE name IN ('gm mountup','modify mount');

INSERT INTO command VALUES
('gm mountup',1,'Syntax: .gm mountup [fast|slow]\r\n\r\nAcquire a random unusual land mount.'),
('modify mount',1,'Syntax: .modify mount [fast|slow]\r\n\r\nProvide selected player a random unusual land mount.');
