-- proc only from tick
UPDATE `spell_proc_event` SET `procFlags` = 2048 WHERE `entry` IN (19572, 19573, 28716, 40438);

-- proc from tick and heal
UPDATE `spell_proc_event` SET `procFlags` = 134219776 WHERE `entry` IN (28744, 33953, 37288);
