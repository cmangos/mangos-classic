ALTER TABLE db_version CHANGE COLUMN required_z1945_s1533_11947_01_mangos_dbscripts required_z1951_s1540_11955_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (555,556);
