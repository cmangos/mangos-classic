ALTER TABLE db_version CHANGE COLUMN required_z2619_s2261_12821_01_mangos_command required_z2621_s2263_12823_01_mangos_command bit;

DELETE FROM command WHERE name='wp import';
