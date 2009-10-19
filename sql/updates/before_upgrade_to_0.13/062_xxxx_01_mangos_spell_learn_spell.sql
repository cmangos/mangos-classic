ALTER TABLE db_version CHANGE COLUMN required_062______01_mangos_spell_learn_spell required_061_7067_02_mangos_spell_learn_spell bit;

DELETE FROM spell_learn_spell WHERE Entry IN (12303,12788,12789);
