ALTER TABLE db_version CHANGE COLUMN required_z1356_s0803_01_mangos_command required_z1357_s0811_01_mangos_spell_threat bit;

DELETE FROM spell_threat WHERE entry IN (845, 7369, 11608, 11609, 20569, 14921, 11775);

INSERT INTO `spell_threat` VALUES
/* Cleave */
(  845,  10, 1, 0),
( 7369,  40, 1, 0),
(11608,  60, 1, 0),
(11609,  70, 1, 0),
(20569, 100, 1, 0);
