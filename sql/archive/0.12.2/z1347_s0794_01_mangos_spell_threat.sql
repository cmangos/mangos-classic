ALTER TABLE db_version CHANGE COLUMN required_z1327_s0658_01_mangos_spell_bonus_data required_z1347_s0794_01_mangos_spell_threat bit;

DELETE FROM spell_threat WHERE entry IN (99, 1735, 9490, 9747, 9898, 20243);

INSERT INTO `spell_threat` VALUES
(   99,  15, 1, 0),
( 1735,  25, 1, 0),
( 9490,  29, 1, 0),
( 9747,  36, 1, 0),
( 9898,  42, 1, 0),
(20243, 101, 1, 0);
