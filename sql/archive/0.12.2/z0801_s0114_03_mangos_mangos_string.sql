ALTER TABLE db_version CHANGE COLUMN required_z0801_s0114_02_mangos_command required_z0801_s0114_03_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (373, 374, 375);

INSERT INTO mangos_string VALUES
(373,'Response:\n%s ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(374,'Tickets count: %i\n',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(375,'Player %s not have tickets.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
