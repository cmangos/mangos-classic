ALTER TABLE db_version CHANGE COLUMN required_z0525_116_02_mangos_creature_movement_scripts required_z0526_117_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (1011,1010,1012,1013,1142);
INSERT INTO mangos_string VALUES
(1010,'| ID         |    Account    |       Character      |       IP        | GM | Expansion |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1012,'========================================================================================',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1013,'| %10u |%15s| %20s | %15s |%4d| %9d |',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1142,'%u - %s (Online:%s IP:%s GM:%u Expansion:%u)',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
