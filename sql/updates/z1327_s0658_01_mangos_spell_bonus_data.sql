ALTER TABLE db_version CHANGE COLUMN required_z1263_s0696_01_mangos_spell_threat required_z1327_s0658_01_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE entry IN (18764);
INSERT INTO spell_bonus_data VALUES
(18764,0,0,0,'Item - Fungal Regrowth');
