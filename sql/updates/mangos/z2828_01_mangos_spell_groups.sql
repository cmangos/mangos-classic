ALTER TABLE db_version CHANGE COLUMN required_z2827_01_mangos_spawn_zone required_z2828_01_mangos_spell_groups bit;

DROP TABLE IF EXISTS `spell_group`;
CREATE TABLE `spell_group`(
  `Id` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Group Identifier',
  `Rule` smallint unsigned NOT NULL,
  `Name` varchar(200) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL COMMENT 'Description of usage',
  PRIMARY KEY(`Id`)
);

DROP TABLE IF EXISTS `spell_group_spell`;
CREATE TABLE `spell_group_spell`(
  `Id` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `SpellId` int unsigned NOT NULL,
  PRIMARY KEY(`Id`,`SpellId`)
);

-- select CONCAT('(1,',Id,'), -- ',SpellName) from spell_template where SpellFamilyName=0 AND AuraInterruptFlags&0x00040000 AND (EffectApplyAuraName1=84 OR EffectApplyAuraName1=20 OR EffectApplyAuraName2=84 OR EffectApplyAuraName2=20 OR EffectApplyAuraName3=84 OR EffectApplyAuraName3=20); - food query
-- SELECT CONCAT('(17,',entry,'), -- ',b.SpellName) FROM spell_elixir a join spell_template b on a.entry=b.Id WHERE Mask&0x01 != 0 order by entry;
-- SELECT CONCAT('(18,',entry,'), -- ',b.SpellName) FROM spell_elixir a join spell_template b on a.entry=b.Id WHERE Mask&0x02 != 0 order by entry;
-- SELECT CONCAT('(3,',entry,'), -- ',b.SpellName) FROM spell_elixir a join spell_template b on a.entry=b.Id WHERE Mask&0x10 != 0;

INSERT INTO spell_group(Id, Rule, Name) VALUES
(1,1,'Food'),
(2,1,'Drink'),
(3,1,'Well Fed'),
(4,1,'Flask Elixir'),
(5,1,'Seal'),
(6,2,'Judgement'),
(7,2,'Blessing'),
(8,2,'Aura'),
(9,2,'Sting'),
(10,2,'Aspect'),
(11,1,'Tracker'),
(12,2,'Curse'),
(13,1,'Mage Armor'),
(14,1,'Warlock Armor'),
(15,1,'Elemental Shield'),
(16,1,'Power Casting Buff'),
(19,1,'Shout Vanilla'),
(21,2,'Alcohol Vanilla-only'),
(23,1,'Soulstone Resurrection');

INSERT INTO spell_group_spell(Id, SpellId) VALUES
-- Food
(1,433), -- Food
(1,434), -- Food
(1,435), -- Food
(1,1127), -- Food
(1,1129), -- Food
(1,1131), -- Food
(1,2639), -- Food
(1,5004), -- Food
(1,5005), -- Food
(1,5006), -- Food
(1,5007), -- Food
(1,6410), -- Food
(1,7737), -- Food
(1,9177), -- Tigule and Foror's Strawberry Ice Cream
(1,10256), -- Food
(1,10257), -- Food
(1,18124), -- Blessed Sunfruit
(1,18229), -- Food
(1,18230), -- Food
(1,18231), -- Food
(1,18232), -- Food
(1,18233), -- Food
(1,18234), -- Food
(1,21149), -- Egg Nog
(1,22731), -- Food
(1,24005), -- Food
(1,24707), -- Food
(1,24800), -- Food
(1,24869), -- Food
(1,25660), -- Food
(1,25695), -- Food
(1,25697), -- Old Enriched Manna Biscuit
(1,25700), -- Food
(1,25702), -- Food
(1,25886), -- Food
(1,25888), -- Food
(1,25990), -- Graccu's Mince Meat Fruitcake
(1,26030), -- Windblossom Berries
(1,26260), -- Food
(1,26263), -- Dim Sum
(1,26401), -- Food
(1,26472), -- Food
(1,26474), -- Food
(1,27094), -- Food
(1,28616), -- Food
(1,29008), -- Food
(1,29038), -- Fizzy Energy Drink
(1,29055), -- Refreshing Red Apple
(1,29073), -- Food

-- Drink
(2,430), -- Drink
(2,431), -- Drink
(2,432), -- Drink
(2,833), -- Green Tea
(2,1133), -- Drink
(2,1135), -- Drink
(2,1137), -- Drink
(2,2639), -- Food
(2,10250), -- Drink
(2,18140), -- Blessed Sunfruit Juice
(2,22734), -- Drink
(2,23698), -- Alterac Spring Water
(2,24355), -- Drink
(2,24707), -- Food
(2,25696), -- Drink
(2,25697), -- Old Enriched Manna Biscuit
(2,25701), -- Brain Food
(2,25703), -- Brain Food
(2,25887), -- Brain Food
(2,25889), -- Brain Food
(2,25990), -- Graccu's Mince Meat Fruitcake
(2,26261), -- Drink
(2,26263), -- Dim Sum
(2,26402), -- Drink
(2,26473), -- Drink
(2,26475), -- Drink
(2,27089), -- Drink
(2,29007), -- Drink
(2,29039), -- Fizzy Energy Drink
(2,29055), -- Refreshing Red Apple

-- Well Fed
(3,18125),
(3,18141),
(3,18191),
(3,18192),
(3,18193),
(3,22730),
(3,25661),

-- attribute query cleaned
(3,128), -- Spellstone
(3,15851), -- Dragonbreath Chili
(3,15852), -- Dragonbreath Chili
(3,17624), -- Petrification
(3,17729), -- Greater Spellstone
(3,17730), -- Major Spellstone
(3,19705), -- Well Fed
(3,19706), -- Well Fed
(3,19708), -- Well Fed
(3,19709), -- Well Fed
(3,19710), -- Well Fed
(3,19711), -- Well Fed
(3,24799), -- Well Fed
(3,24870), -- Well Fed
(3,24991), -- Elemental Slayer
(3,25694), -- Well Fed
(3,25851), -- Lightheaded
(3,25941), -- Well Fed
(3,28170), -- Master Spellstone
(3,29040), -- Fizzy Energy Drink
(3,29220), -- Fiery Festival Brew
(3,29221), -- Fiery Festival Brew

-- Flask
(4,17626),
(4,17628),
(4,17627),
(4,17629),

-- Seal
(5,20154), -- Seal of Righteousness
(5,20287),
(5,20288),
(5,20289),
(5,20290),
(5,20291),
(5,20292),
(5,20293),
(5,21084),
(5,20162), -- Seal of the Crusader
(5,20305),
(5,20306),
(5,20307),
(5,20308),
(5,21082),
(5,20164), -- Seal of Justice
(5,20165), -- Seal of Light
(5,20347),
(5,20348),
(5,20349),
(5,20166), -- Seal of Wisdom
(5,20356),
(5,20357),
(5,20375), -- Seal of Command
(5,20915),
(5,20918),
(5,20919),
(5,20920),

-- Judgement
(6,20184), -- Judgement of Justice
(6,20185), -- Judgement of Light
(6,20344),
(6,20345),
(6,20346),
(6,20186), -- Judgement of Wisdom
(6,20354),
(6,20355),
(6,21183), -- Judgement of the Crusader
(6,20188),
(6,20300),
(6,20301),
(6,20302),
(6,20303),

-- Blessing
(7,1022), -- Blessing of Protection
(7,5599),
(7,10278),
(7,1038), -- Blessing of Salvation
(7,25895), -- Greater Blessing of Salvation
(7,6940), -- Blessing of Sacrifice
(7,20729),
(7,19740), -- Blessing of Might
(7,19834),
(7,19835),
(7,19836),
(7,19837),
(7,19838),
(7,25291),
(7,25782), -- Greater Blessing of Might
(7,25916),
(7,19742), -- Blessing of Wisdom
(7,19850),
(7,19852),
(7,19853),
(7,19854),
(7,25290),
(7,25894), -- Greater Blessing of Wisdom
(7,25918),
(7,27143),
(7,19977), -- Blessing of Light
(7,19978),
(7,19979),
(7,25890), -- Greater Blessing of Light
(7,20217), -- Blessing of Kings
(7,25898), -- Greater Blessing of Kings
(7,20911), -- Blessing of Sanctuary
(7,20912),
(7,20913),
(7,20914),
(7,25899), -- Greater Blessing of Sanctuary

-- Aura
(8,465), -- Devotion Aura
(8,10290),
(8,643),
(8,10291),
(8,1032),
(8,10292),
(8,10293),
(8,7294), -- Retribution Aura
(8,10298),
(8,10299),
(8,10300),
(8,10301),
(8,19746), -- Concentration Aura
(8,19876), -- Shadow Resistance Aura
(8,19895),
(8,19896),
(8,19888), -- Frost Resistance Aura
(8,19897),
(8,19898),
(8,19891), -- Fire Resistance Aura
(8,19899),
(8,19900),
(8,20218), -- Sanctity Aura

-- Sting
(9,1978), -- serpent sting
(9,13549),
(9,13550),
(9,13551),
(9,13552),
(9,13553),
(9,13554),
(9,13555),
(9,25295),
(9,3034), -- viper sting
(9,14279),
(9,14280),
(9,3043), -- scorpid sting
(9,19386), -- Wyvern Sting
(9,24131),
(9,24132),
(9,24133),
(9,24134),
(9,24135),

-- aspect
(10,13161), -- Aspect of the Beast
(10,5118), -- Aspect of the Cheetah
(10,13159), -- Pack
(10,13163), -- Monkey
(10,13165), -- Hawk
(10,14318),
(10,14319),
(10,14320),
(10,14321),
(10,14322),
(10,25296),
(10,20043), -- Wild
(10,20190),

-- Tracker
(11,1494), -- Track Beasts
(11,2383), -- Find Herbs
(11,2481), -- Find Treasure
(11,2580), -- Find Minerals
(11,5225), -- Track Humanoids
(11,5500), -- Sense Demons
(11,5502), -- Sense Undead
(11,11389), -- Detect Undead
(11,11407), -- Detect Demon
(11,19878), -- Track Demons
(11,19879), -- Track Dragonkin
(11,19880), -- Track Elementals
(11,19882), -- Track Giants
(11,19883), -- Track Humanoids
(11,19884), -- Track Undead
(11,19885), -- Track Hidden

-- Curse
(12,603), -- doom
(12,30910),
(12,702), -- weakness
(12,1108),
(12,6205),
(12,7646),
(12,11707),
(12,11708),
(12,704), -- recklessness
(12,7658),
(12,7659),
(12,11717),
(12,1714), -- tongues
(12,11719),
(12,980), -- agony
(12,1014),
(12,6217),
(12,11711),
(12,11712),
(12,11713),
(12,1490), -- elements
(12,11721),
(12,11722),
(12,17862), -- shadow
(12,17937),

-- Mage Armor
(13,168), -- frost armor
(13,7300),
(13,7301),
(13,7302), -- ice armor
(13,7320),
(13,10219),
(13,10220),
(13,6117), -- mage armor
(13,22782),
(13,22783),

-- Warlock armor
(14,687), -- demon skin
(14,696),
(14,706), -- demon armor
(14,1086),
(14,11733),
(14,11734),
(14,11735),

-- Elemental Shield
(15,23552), -- T2 8 bonus
(15,324), -- Lightning Shield
(15,325),
(15,905),
(15,945),
(15,8134),
(15,10431),
(15,10432),

-- Power Casting Buff
(16,12042), -- Arcane Power
(16,10060), -- Power Infusion

-- Shout
(19,6673), -- battle shout
(19,5242),
(19,6192),
(19,11549),
(19,11550),
(19,11551),
(19,25289),

-- Soulstone Resurrection
(23,20707),
(23,20762),
(23,20763),
(23,20764),
(23,20765);

