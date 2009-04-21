-- add PROC_FLAG_PERIODIC_TICK to PROC_FLAG_HEAL
UPDATE `spell_proc_event` SET `procFlags` = 0x08000800 WHERE `entry` = 45484;

-- delete old TEST spell
DELETE FROM `spell_proc_event` WHERE `entry` = 18189;
