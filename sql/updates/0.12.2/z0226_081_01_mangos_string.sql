ALTER TABLE db_version CHANGE COLUMN required_z0183_093_01_mangos_command required_z0226_081_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN(60,61,62);
INSERT INTO mangos_string VALUES
(60,'I\'m busy right now, come back later.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(61,'Username: ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(62,'Password: ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
