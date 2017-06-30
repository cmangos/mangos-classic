ALTER TABLE db_version CHANGE COLUMN required_z2700_01_mangos_paladin_bonus_scaling required_z2701_01_mangos_spell_chain_sor bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN (21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293);
INSERT INTO `spell_chain` VALUES
/* Seal of Righteousness */
(21084,0,21084,1,0),
(20287,21084,21084,2,0),
(20288,20287,21084,3,0),
(20289,20288,21084,4,0),
(20290,20289,21084,5,0),
(20291,20290,21084,6,0),
(20292,20291,21084,7,0),
(20293,20292,21084,8,0);
