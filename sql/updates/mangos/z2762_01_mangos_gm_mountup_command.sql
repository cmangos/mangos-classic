ALTER TABLE db_version CHANGE COLUMN required_z2761_01_mangos_mount_commands required_z2762_01_mangos_gm_mountup_command bit;

DELETE FROM command WHERE name IN ('gm mountup');

INSERT INTO command VALUES
('gm mountup',1,'Syntax: .gm mountup [fast|slow|#displayid|target]\r\n\r\nIf #displayid is provided, visually mounts your character on a provided creature likeness. If your target is a creature and corresponding arg is provided, visually mounts your character on the likeness of targeted creature. In other cases this command mounts your character on a random unusual land mount according to provided or omitted args.');
