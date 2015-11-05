-- proc only from tick
UPDATE `spell_proc_event` SET `procFlags` = 0x00000800 WHERE `entry` IN (18094, 18095, 32385, 32387, 32392, 32393, 32394, 33191, 33192, 33193, 33194, 33195, 38394, 40478, 45054);

-- proc from tick and direct hit
UPDATE `spell_proc_event` SET `procFlags` = 0x00020800 WHERE `entry` IN (18119, 18120, 18121, 18122, 18123, 39372, 45481);

-- proc on melee, ranged and spell hit (+ tick proc added)
UPDATE `spell_proc_event` SET `procFlags` = 0x000A0801 WHERE `entry` = 37381;
