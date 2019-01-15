ALTER TABLE db_version CHANGE COLUMN required_z2730_01_mangos_seal_of_righteousness_restored required_z2731_01_mangos_seal_of_righteousness_proc_restored bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN (25742, 25741, 25740, 25739, 25738, 25737, 25736, 25735, 25713);
INSERT INTO `spell_chain` VALUES
/* Seal of Righteousness Proc */
(25742,0,25742,1,0),
(25741,25742,25742,2,0),
(25740,25741,25742,3,0),
(25739,25740,25742,4,0),
(25738,25739,25742,5,0),
(25737,25738,25742,6,0),
(25736,25737,25742,7,0),
(25735,25736,25742,8,0),
(25713,25735,25742,9,0);

