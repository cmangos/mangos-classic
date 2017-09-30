ALTER TABLE db_version CHANGE COLUMN required_z2701_01_mangos_spell_chain_sor required_z2702_01_mangos_spell_chain_totems_typos bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN (10534);
INSERT INTO `spell_chain` VALUES
(10534,8185,8185,2,0);
