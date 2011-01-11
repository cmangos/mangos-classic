ALTER TABLE db_version CHANGE COLUMN required_z1218_s0638_01_mangos_spell_bonus_data required_z1218_s0638_02_mangos_spell_proc_event bit;

DELETE FROM spell_proc_event WHERE entry IN (12880);
INSERT INTO `spell_proc_event` VALUES
(12880, 0x00,  0, 0x0000000000000000, 0x00000000, 0x00010000, 0.000000, 0.000000,  0);
