ALTER TABLE db_version CHANGE COLUMN required_z2828_01_mangos_spell_groups required_z2829_01_mangos_proc_cooldown bit;

UPDATE spell_proc_event SET cooldown=cooldown*1000 WHERE entry BETWEEN 0 AND 100000;