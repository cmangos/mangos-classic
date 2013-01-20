ALTER TABLE db_version CHANGE COLUMN required_z2054_s1667_12097_01_mangos_mangos_string required_z2067_sxxxx_xxxxx_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (1608, 1609, 1635, 1636);

INSERT INTO mangos_string VALUES
(1635,'|cffffff00The Horde has collected 200 silithyst!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1636,'|cffffff00The Alliance has collected 200 silithyst!|r',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
