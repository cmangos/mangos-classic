ALTER TABLE db_version CHANGE COLUMN required_z1527_s1022_01_mangos_mangos_string required_z1529_s1024_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (274);

INSERT INTO mangos_string VALUES
(274,'Game Object (GUID: %u) has references in not found owner %s GO list, can\'t be deleted.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
