ALTER TABLE db_version CHANGE COLUMN required_z1486_s0972_01_mangos_spell_bonus_data required_z1488_s0974_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (1165);

INSERT INTO mangos_string VALUES
(1165,'Spell %u not have auras.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
