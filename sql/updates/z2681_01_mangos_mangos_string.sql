ALTER TABLE db_version CHANGE COLUMN required_z2680_01_mangos_battleground_template required_z2681_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry=58;

INSERT INTO mangos_string VALUES
(58,'Using script library',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
