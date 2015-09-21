ALTER TABLE db_version CHANGE COLUMN required_z1192_s0612_01_mangos_spell_proc_event required_z1192_s0612_02_mangos_spell_chain bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN (12880,14201,14202,14203,14204);
INSERT INTO `spell_chain` (`spell_id`, `prev_spell`, `first_spell`, `rank`, `req_spell`) VALUES
(12880,0,12880,1,0),   -- Enrage
(14201,12880,12880,2,0),
(14202,14201,12880,3,0),
(14203,14202,12880,4,0),
(14204,14203,12880,5,0);
