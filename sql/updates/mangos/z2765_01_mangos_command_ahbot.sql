ALTER TABLE db_version CHANGE COLUMN required_z2763_01_mangos_ability_sets required_z2765_01_mangos_command_ahbot bit;

DELETE FROM command WHERE name LIKE 'ahbot%';
