ALTER TABLE db_version CHANGE COLUMN required_z2729_01_mangos_creature_template_faction_removal required_z2730_01_mangos_seal_of_righteousness_restored bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN (20154, 21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293);
INSERT INTO `spell_chain` VALUES
/* Seal of Righteousness (serverside extension) */
(20287,21084,20154,3,0),
(20288,20287,20154,4,0),
(20289,20288,20154,5,0),
(20290,20289,20154,6,0),
(20291,20290,20154,7,0),
(20292,20291,20154,8,0),
(20293,20292,20154,9,0);

UPDATE `playercreateinfo_action` SET `action` = 20154 WHERE `action` = 21084 AND `type` = 0;

UPDATE `playercreateinfo_spell` SET `Spell` = 20154 WHERE `Spell` = 21084;

UPDATE `npc_trainer_template` SET `spell` = 10321 WHERE `spell` = 20271;

UPDATE `npc_trainer` SET `spell` = 10321 WHERE `spell` = 20271;
