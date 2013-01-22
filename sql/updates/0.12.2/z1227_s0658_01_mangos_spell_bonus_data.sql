ALTER TABLE db_version CHANGE COLUMN required_z1225_s0654_01_mangos_mangos_string required_z1227_s0658_01_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE entry IN (5707,17712);
INSERT INTO spell_bonus_data VALUES
(5707,0,0,0,'Item - Lifestone Regeneration'),
(17712,0,0,0,'Item - Lifestone Healing');
