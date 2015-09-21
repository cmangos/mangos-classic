ALTER TABLE db_version CHANGE COLUMN required_z1800_s1356_11731_02_mangos_mangos_string required_z1840_s1399_11813_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (707);

INSERT INTO mangos_string VALUES (707,'%s does not wish to be disturbed: %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
