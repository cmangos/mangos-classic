ALTER TABLE db_version CHANGE COLUMN required_061_7067_02_mangos_spell_learn_spell required_062______01_mangos_spell_learn_spell bit;

DELETE FROM spell_learn_spell WHERE Entry IN (12303,12788,12789);

INSERT INTO spell_learn_spell VALUES
(12303,45471,0),
(12788,45471,0),
(12789,45471,0);
