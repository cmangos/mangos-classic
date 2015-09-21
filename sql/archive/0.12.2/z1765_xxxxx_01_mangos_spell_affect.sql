ALTER TABLE db_version CHANGE COLUMN required_z1756_s1308_01_mangos_instance_template required_z1765_xxxxx_01_mangos_spell_affect bit;

DELETE FROM `spell_affect` WHERE `entry` IN (18275, 18274, 18273, 18272, 18271) AND effectId = 1;
INSERT INTO spell_affect ( entry , effectId , SpellFamilyMask ) VALUES
(18275, 1, 4295492618),
(18274, 1, 4295492618),
(18273, 1, 4295492618),
(18272, 1, 4295492618),
(18271, 1, 4295492618);
