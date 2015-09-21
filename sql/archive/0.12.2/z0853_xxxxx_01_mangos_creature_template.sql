ALTER TABLE db_version CHANGE COLUMN required_z0831_s0150_01_mangos_spell_chain required_z0853_xxxxx_01_mangos_creature_template bit;

UPDATE creature_template SET spell1 = 0 WHERE entry = 3968;