ALTER TABLE db_version CHANGE COLUMN required_z1683_s1231_01_mangos_creature_template required_z1695_s1244_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (8);
INSERT INTO mangos_string VALUES
(8,'Command %s have subcommands:',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
