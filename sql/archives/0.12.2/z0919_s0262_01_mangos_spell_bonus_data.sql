ALTER TABLE db_version CHANGE COLUMN required_z0908_s0251_01_mangos_command required_z0919_s0262_01_mangos_spell_bonus_data bit;

DELETE FROM `spell_bonus_data` WHERE `entry` IN (8936, 139, 596, 15407, 2060, 2061, 13908, 10461, 10460, 6372, 6371, 5672, 974, 8004, 331, 1454);
INSERT INTO `spell_bonus_data`(`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`comments`) VALUES
( 8936, 0.3,    0.1,    0, 'Druid - Regrowth'),
(  139, 0,      0.2,    0, 'Priest - Renew'),
(  596, 0.4285, 0,      0, 'Priest - Prayer of Healing'),
(15407, 0,      0.19,   0, 'Priest - Mind Flay'),
( 2060, 1.2353, 0,      0, 'Priest - Greater Heal'),
( 2061, 0.6177, 0,      0, 'Priest - Flash Heal'),
(13908, 0.4286, 0,      0, 'Priest - Desperate Prayer'),
(  974, 0.2857, 0,      0, 'Shaman - Earth Shield'),
( 5672, 0,      0.0450, 0, 'Shaman - Healing Stream Totem'),
(  331, 0.8571, 0,      0, 'Shaman - Healing Wave'),
( 8004, 0.4286, 0,      0, 'Shaman - Lesser Healing Wave'),
( 1454, 0.8,    0,      0, 'Warlock - Life Tap');
