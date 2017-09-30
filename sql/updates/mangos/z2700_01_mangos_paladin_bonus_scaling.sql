ALTER TABLE db_version CHANGE COLUMN required_z2699_01_mangos_spell_chain_totems required_z2700_01_mangos_paladin_bonus_scaling bit;

DELETE FROM `spell_bonus_data` WHERE `entry` IN (20911, 25997, 25899, 19968, 20187, 26573, 879, 24275, 20925, 2812, 20424,20167, 25742);
INSERT INTO `spell_bonus_data` VALUES
(20911, 0,      0,       0,     0,     'Paladin - Blessing of Sanctuary'),
(25997, 1,      0,       0,     0,     'Paladin - Eye for an Eye'),
(25899, 0,      0,       0,     0,     'Paladin - Greater Blessing of Sanctuary'),
(19968, 0.7143, 0,       0,     0,     'Paladin - Holy Light'),
(20925, 0.05,   0,       0,     0,     'Paladin - Holy Shield'),
(2812,  0.1905, 0,       0,     0,     'Paladin - Holy Wrath'),
(20187, 0.5,    0,       0,     0,     'Paladin - Judgement of Righteousness'),
(20424, 0.2,    0,       0,     0,     'Paladin - Seal of Command Proc'),
(25742, 0.1,    0,       0,     0,     'Paladin - Seal of Righteousness Proc');

DELETE FROM `spell_chain` WHERE `spell_id` IN (20187, 20280, 20281, 20282, 20283, 20284, 20285, 20286, 25742, 25740, 25739, 25738, 25737, 25736, 25735, 25713);
INSERT INTO `spell_chain` VALUES
/* Judgement of Righteousness */
(20187,0,20187,1,0),
(20280,20187,20187,2,0),
(20281,20280,20187,3,0),
(20282,20281,20187,4,0),
(20283,20282,20187,5,0),
(20284,20283,20187,6,0),
(20285,20284,20187,7,0),
(20286,20285,20187,8,0),
/* Seal of Righteousness Proc */
(25742,0,25742,1,0),
(25740,25742,25742,2,0),
(25739,25740,25742,3,0),
(25738,25739,25742,4,0),
(25737,25738,25742,5,0),
(25736,25737,25742,6,0),
(25735,25736,25742,7,0),
(25713,25735,25742,8,0);
