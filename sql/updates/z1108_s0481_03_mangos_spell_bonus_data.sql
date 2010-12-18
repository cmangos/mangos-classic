ALTER TABLE db_version CHANGE COLUMN required_z1108_s0481_02_mangos_spell_chain required_z1108_s0481_03_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE entry IN (8037, 10458, 16352, 16353);
