ALTER TABLE db_version CHANGE COLUMN required_z2612_s2254_12816_02_mangos_mangos_string required_z2619_s2261_12821_01_mangos_command bit;

DELETE FROM command WHERE name='npc addmove';
