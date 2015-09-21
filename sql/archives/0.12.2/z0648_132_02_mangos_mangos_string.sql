ALTER TABLE db_version CHANGE COLUMN required_z0648_132_01_mangos_command required_z0648_132_02_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (269);
