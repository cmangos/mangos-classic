ALTER TABLE db_version CHANGE COLUMN required_z0967_s0263_01_mangos_mangos_string required_z0968_s0283_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry >= 667 and entry <= 687 or entry IN (614,615,755);
INSERT INTO mangos_string VALUES
(614,'The Alliance flag is now placed at its base.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(615,'The Horde flag is now placed at its base.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
