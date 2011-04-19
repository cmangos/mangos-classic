ALTER TABLE db_version CHANGE COLUMN required_z1482_s0967_01_mangos_spell_proc_event required_z1486_s0972_01_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE entry = 10444;
INSERT INTO spell_bonus_data VALUES (10444, 0, 0, 0, "Shaman - Flametongue Attack");
