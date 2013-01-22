ALTER TABLE db_version CHANGE COLUMN required_z1439_s0915_01_mangos_creature_addon required_z1442_s0923_01_mangos_spell_affect bit;

DELETE FROM `spell_affect` WHERE `entry` IN (11170,12982,12983,12984,12985);
INSERT INTO spell_affect ( entry , effectId , SpellFamilyMask ) VALUES
(11170, 0, 0x0000000040FE1AF7),
(12982, 0, 0x0000000040FE1AF7),
(12983, 0, 0x0000000040FE1AF7),
(12984, 0, 0x0000000040FE1AF7),
(12985, 0, 0x0000000040FE1AF7);


