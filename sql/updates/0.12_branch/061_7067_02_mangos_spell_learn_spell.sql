ALTER TABLE db_version CHANGE COLUMN required_061_7067_01_mangos_playercreateinfo_spell required_061_7067_02_mangos_spell_learn_spell bit;

ALTER TABLE spell_learn_spell
  ADD COLUMN Active tinyint(3) unsigned NOT NULL default '1' AFTER SpellID;

DELETE FROM spell_learn_spell WHERE Entry IN (17002,24866,33872,33873);

INSERT INTO spell_learn_spell VALUES
(17002,24867,0),
(24866,24864,0),
(33872,47179,0),
(33873,47180,0);
