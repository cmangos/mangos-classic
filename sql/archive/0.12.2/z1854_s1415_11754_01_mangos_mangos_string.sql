ALTER TABLE db_version CHANGE COLUMN required_z1852_s1413_11827_01_mangos_creature_linking_template required_z1854_s1415_11754_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (1192);

INSERT INTO mangos_string VALUES (1192,'Effect movement',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
