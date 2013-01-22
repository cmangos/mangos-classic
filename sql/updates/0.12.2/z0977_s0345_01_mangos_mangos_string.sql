ALTER TABLE db_version CHANGE COLUMN required_z0968_s0283_01_mangos_mangos_string required_z0977_s0345_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (201);

INSERT INTO mangos_string VALUES
(201,'Object GUID is: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
