ALTER TABLE db_version CHANGE COLUMN required_z2732_01_mangos_seal_of_righteousness_cleanup required_z2733_01_mangos_playercreate_skills_vanilla bit;

DROP TABLE IF EXISTS `playercreateinfo_skills`;

CREATE TABLE `playercreateinfo_skills` (
`raceMask` INT UNSIGNED NOT NULL,
`classMask` INT UNSIGNED NOT NULL,
`skill` SMALLINT(5) UNSIGNED NOT NULL,
`step` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
`note` VARCHAR(255) DEFAULT NULL,
PRIMARY KEY (`raceMask`,`classMask`,`skill`)
)
ENGINE=MyISAM DEFAULT CHARSET=UTF8;

INSERT INTO `playercreateinfo_skills` VALUES
-- ALL PLAYERS:
(0,     0,  95, 0,  'Misc: Defense'),
(0,     0, 162, 0,  'Weapon: Unarmed'),
(0,     0, 183, 0,  'Misc: GENERIC (DND)'),
(0,     0, 415, 0,  'Armor: Cloth'),
-- WARRIOR CLASS:
(0,     1,  26, 0,  'Warrior: Arms'),
(0,     1, 256, 0,  'Warrior: Fury'),
(0,     1, 257, 0,  'Warrior: Protection'),
-- PALADIN CLASS:
(0,     2, 594, 0,  'Paladin: Holy'),
(0,     2, 267, 0,  'Paladin: Protection'),
(0,     2, 184, 0,  'Paladin: Retribution'),
-- HUNTER CLASS:
(0,     4,  50, 0,  'Hunter: Beast Mastery'),
(0,     4, 163, 0,  'Hunter: Marksmanship'),
(0,     4,  51, 0,  'Hunter: Survival'),
-- ROGUE CLASS:
(0,     8,  38, 0,  'Rogue: Combat'),
(0,     8, 253, 0,  'Rogue: Assassination'),
(0,     8,  39, 0,  'Rogue: Subtlety'),
(0,     8, 176, 0,  'Weapon: Thrown'),
(0,     8, 118, 0,  'Misc: Dual Wield'),
-- PRIEST CLASS:
(0,    16,  56, 0,  'Priest: Holy'),
(0,    16, 613, 0,  'Priest: Discipline'),
(0,    16,  78, 0,  'Priest: Shadow'),
-- SHAMAN CLASS:
(0,    64, 375, 0,  'Shaman: Elemental'),
(0,    64, 373, 0,  'Shaman: Enhancement'),
(0,    64, 374, 0,  'Shaman: Restoration'),
-- MAGE CLASS:
(0,   128, 237, 0,  'Mage: Arcane'),
(0,   128,   8, 0,  'Mage: Fire'),
(0,   128,   6, 0,  'Mage: Frost'),
-- WARLOCK CLASS:
(0,   256, 355, 0,  'Warlock: Affliction'),
(0,   256, 354, 0,  'Warlock: Demonology'),
(0,   256, 593, 0,  'Warlock: Destruction'),
-- DRUID CLASS:
(0,  1024, 574, 0,  'Druid: Balance'),
(0,  1024, 134, 0,  'Druid: Feral Combat'),
(0,  1024, 573, 0,  'Druid: Restoration'),
-- WEAPONS, ARMOR AND MISC SKILLS BY CLASSES:
(0,     5,  44, 0,  'Weapon: Axes'),
(0,     5, 172, 0,  'Weapon: Two-Handed Axes'),
(0,     7,  43, 0,  'Weapon: Swords'),
(0,     3,  55, 0,  'Weapon: Two-Handed Swords'),
(0,  1107,  54, 0,  'Weapon: Maces'),
(0,  1488, 136, 0,  'Weapon: Staves'),
(0,   400, 228, 0,  'Weapon: Wands'),
(0,     3, 413, 0,  'Armor: Mail'),
(0,  1103, 414, 0,  'Armor: Leather'),
(0,    67, 433, 0,  'Armor: Shield'),
-- ALLIANCE RACES:
(1,     0, 754, 0,  'Racial: Human'),
(77,    0,  98, 0,  'Language: Common'),
(4,     0, 101, 0,  'Racial: Dwarf'),
(4,     0, 111, 0,  'Language: Dwarven'),
(8,     0, 126, 0,  'Racial: Night Elf'),
(8,     0, 113, 0,  'Language: Darnassian'),
(64,    0, 753, 0,  'Racial: Gnome'),
(64,    0, 313, 0,  'Language: Gnomish'),
-- HORDE RACES:
(2,     0, 125, 0,  'Racial: Orc'),
(178,   0, 109, 0,  'Language: Orcish'),
(16,    0, 220, 0,  'Racial: Undead'),
(16,    0, 673, 0,  'Language: Gutterspeak'),
(32,    0, 124, 0,  'Racial: Tauren'),
(32,    0, 115, 0,  'Language: Taurahe'),
(128,   0, 733, 0,  'Racial: Troll'),
(128,   0, 315, 0,  'Language: Troll'),
-- WEAPONS, ARMOR AND MISC SKILLS BY RACES AND CLASSES:
(37,    3, 160, 0,  'Weapon: Two-Handed Maces'),
(223,1293, 173, 0,  'Weapon: Daggers'),
(36,    4,  46, 0,  'Weapon: Guns'),
(138,   4,  45, 0,  'Weapon: Bows');

