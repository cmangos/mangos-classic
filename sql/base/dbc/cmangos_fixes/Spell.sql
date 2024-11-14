
-- Spell 24148 should have passive attribute. Without it, items with that spell enchant are declared as invalid when logging in a dead player
-- Confirmed on later version of the spell
UPDATE `spell_template` SET `Attributes`=Attributes|0x040 WHERE `Id`=24148;

ALTER TABLE `spell_template` ADD COLUMN `AttributesServerside`  int(11) unsigned NOT NULL DEFAULT '0';

-- Removes aura interrupt flag by sitting for spells that do not make a creature sit, all seem to be copypasted for unknown reasons
UPDATE `spell_template` SET `AuraInterruptFlags`=AuraInterruptFlags&~0x00040000 WHERE `Id` IN(6606,14915,16093);

-- all ranks except rank 3 have mask
UPDATE `spell_template` SET `EffectItemType2`=268435456 WHERE `Id`=16106;
-- all ranks except rank 5 have mask
UPDATE `spell_template` SET `EffectItemType1`=0x00240000 WHERE `Id`=12842;

-- Allow caster to cast this self-target spell even if he is immune to the spell damage school
UPDATE `spell_template` SET `Attributes`=Attributes|0x20000000 WHERE `id` IN(22972,22975,22976,22977,22978,22979,22980,22981,22982,22983,22984,22985,22986);

-- Fixed radius for the various spells used in summoning NPCs in Nefarian encounter
-- Value in DBC made the adds spawn anywhere instead of near their spawner NPC
UPDATE `spell_template` SET `EffectRadiusIndex1`=7 WHERE `id` IN(22654,22655,22656,22657,22658,22680);

-- Fixes spell 13166 (Gnomish Battle Chicken): the summoning type was wrong
UPDATE `spell_template` SET `Effect1`=42, `EffectBasePoints1`=0 WHERE `id`=13166;

-- Fixes InterruptFlags for spell 18115 (Viewing Room Student Transform - Effect): it should not cancel on movement
UPDATE `spell_template` SET `InterruptFlags`=0 WHERE `id`=18115;

-- Fixes Well Fed buff for some seasonal food like Winter Veil cookies
UPDATE `spell_template` SET `Effect2`=6, `EffectDieSides2`=1, `EffectBaseDice2`=1, `EffectImplicitTargetA2`=1, `EffectApplyAuraName2`=23, `EffectAmplitude2`=10000, `EffectTriggerSpell2`=24870 WHERE `Id`=24869;
UPDATE `spell_template` SET `EffectRealPointsPerLevel1`=0.25, `EffectRealPointsPerLevel2`=0.25, `EffectBaseDice1`=0, `EffectBaseDice2`=0, `EffectBasePoints2`=0 WHERE `id`=24870;

-- Added Ignore LoS attribute for Magic Wings spell (trigger NPC is not always in LoS of player)
UPDATE `spell_template` SET `AttributesEx2`=AttributesEx2|0x00000004 WHERE `id`=24742;

-- Remove channeled extra attribute from spell 28370 (Toxic Gas) that is not channeled
UPDATE `spell_template` SET `AttributesEx`=0 WHERE `id`=28370;

-- Hurricane Spell From Druid And NPC must be interruptable
UPDATE `spell_template` SET `InterruptFlags`=15 WHERE `id`=16914 OR `id`=17401 OR `id`=17402;

-- Stun Self - vanilla spell - 44355 - Fel Crystal Visual - entry 24722 - has SPELL_ATTR_EX5_USABLE_WHILE_STUNNED confirmed with sniffs
UPDATE spell_template SET Mechanic=12 WHERE Id IN(25900);

-- sweeping strikes should not be lost on stance change to defensive
UPDATE spell_template SET Attributes=Attributes|0x00010000 WHERE Id IN(12292);

-- targeting section
-- Draw Spirit - Lethon - both effects need to have same radius index - faulty data
UPDATE spell_template SET EffectRadiusIndex1=EffectRadiusIndex2 WHERE Id IN(24811);
-- Opening Termite Barrel - target flags for GO/ITEM, effect targets GO/ITEM yet target targets units - fixes apparent typo
UPDATE spell_template SET EffectImplicitTargetA1=26 WHERE Id IN(18952);
-- Talents - Improved Scorch and Shadow Weaving have TARGET_UNIT_ENEMY, yet the auras are placed on caster anyway
-- It is possible talents are supposed to bypass normal casting, but evidenced by wotlk, the rest are all caster targeted, so assuming copypaste wrong data possibility as well
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(11095,12872,12873,15257,15331,15332,15333,15334);
-- Improved Wing Clip - talent aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(19228,19232,19233,19234,19235);
-- Entrapment - talent aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(19184,19387,19388,19389,19390);
-- Improved Hamstring - talent aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(12289,12668,23695);
-- Celestial Focus - talent aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(16850,16923,16924);
-- Ruthlessness - talent aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(14156,14160);
-- get rid of targeting info for indices with no effect - core optimization
UPDATE spell_template SET EffectImplicitTargetA1=0,EffectImplicitTargetB1=0 WHERE Effect1=0;
UPDATE spell_template SET EffectImplicitTargetA2=0,EffectImplicitTargetB2=0 WHERE Effect2=0;
UPDATE spell_template SET EffectImplicitTargetA3=0,EffectImplicitTargetB3=0 WHERE Effect3=0;
-- vanilla only - Blood Frenzy - talent targets self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(16952,16954);
-- vanilla only - seal fate
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(14186,14190,14193,14194,14195);
-- Sticky Tar - proc aura meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(14178);

-- Mangletooth - These spells need to ignore LOS
UPDATE spell_template SET AttributesEx2 = AttributesEx2|4 WHERE id IN (7764, 16618, 10767, 16610, 16612, 17013);

-- Fix radius of summoning spells for Corpse Scarabs used for Anub'Rekhan
UPDATE spell_template SET EffectRadiusIndex1=16 WHERE id IN (28864, 29105);

-- Fix radius of Consumption spell for Void Zone in Four Horsemen (use same value than TBC DBCs)
UPDATE spell_template SET EffectRadiusIndex1=7 WHERE id=28865;

-- Fix radius of Summon Spore spell in Loatheb (use same value than WotLK DBCs)
UPDATE spell_template SET EffectRadiusIndex1=23 WHERE id=29234;

-- Remove incorrect spell attribute for spell 30122 (Plague Cloud) used in Heigan the Unclean encounter
UPDATE spell_template SET AttributesEx=(AttributesEx&~0x00000040) WHERE id=30122;

-- Custom spells to be used with .modify commands. These spell ids are free in all expansions.
INSERT INTO `spell_template` (`Id`, `Attributes`, `CastingTimeIndex`, `ProcChance`, `SpellLevel`, `DurationIndex`, `EquippedItemClass`, `Effect1`, `EffectApplyAuraName1`, `EffectMiscValue1`, `SpellName`) VALUES
(15170, 64, 1, 101, 1, 21, -1, 6, 22, 1, 'Custom QA Mod Armor'),
(15171, 64, 1, 101, 1, 21, -1, 6, 22, 2, 'Custom QA Mod Resist Holy'),
(15172, 64, 1, 101, 1, 21, -1, 6, 22, 4, 'Custom QA Mod Resist Fire'),
(15173, 64, 1, 101, 1, 21, -1, 6, 22, 8, 'Custom QA Mod Resist Nature'),
(15174, 64, 1, 101, 1, 21, -1, 6, 22, 16, 'Custom QA Mod Resist Frost'),
(15175, 64, 1, 101, 1, 21, -1, 6, 22, 32, 'Custom QA Mod Resist Shadow'),
(15176, 64, 1, 101, 1, 21, -1, 6, 22, 64, 'Custom QA Mod Resist Arcane'),
(15177, 64, 1, 101, 1, 21, -1, 6, 99, 0, 'Custom QA Mod Melee AP'),
(15178, 64, 1, 101, 1, 21, -1, 6, 124, 0, 'Custom QA Mod Ranged AP'),
(15179, 64, 1, 101, 1, 21, -1, 6, 52, 0, 'Custom QA Mod Melee Crit'),
(15180, 64, 1, 101, 1, 21, -1, 6, 57, 0, 'Custom QA Mod Spell Crit'),
(15181, 64, 1, 101, 1, 21, -1, 6, 138, 0, 'Custom QA Mod Melee Haste'),
(15182, 64, 1, 101, 1, 21, -1, 6, 140, 0, 'Custom QA Mod Ranged Haste'),
(15183, 64, 1, 101, 1, 21, -1, 6, 65, 0, 'Custom QA Mod Spell Haste'),
(15184, 64, 1, 101, 1, 21, -1, 6, 47, 0, 'Custom QA Mod Parry Chance'),
(15185, 64, 1, 101, 1, 21, -1, 6, 49, 0, 'Custom QA Mod Dodge Chance'),
(15186, 64, 1, 101, 1, 21, -1, 6, 51, 0, 'Custom QA Mod Block Chance');

-- Razorgore - Destroy Egg - should not be affected by pushback and interrupt on damage
UPDATE spell_template SET InterruptFlags=InterruptFlags&~0x12 WHERE Id IN(19873);

-- Onyxia - Wing buffet is interruptible by things but shouldnt
UPDATE spell_template SET InterruptFlags=0 WHERE Id IN(18500);

-- AQ40 - Ouro - Sweep is interruptible by things but shouldnt
UPDATE spell_template SET InterruptFlags=0 WHERE Id IN(26103);

-- Fix Felguard Destroyer 18977 Sweeping Charge Id: 96 (SPELL_EFFECT_CHARGE) not working correctly changing to Id: 149 (SPELL_EFFECT_CHARGE_DEST)
UPDATE `spell_template` SET `Effect1` = 149 WHERE `Id` IN (33971);

-- Nether Beam - Netherspite - restricted to one target
UPDATE spell_template SET MaxAffectedTargets=1 WHERE Id IN(30469);

-- Fix duration of spell 28561 (Summon Blizzard): NPC should despawn after 30 seconds, not 5 minutes
UPDATE spell_template SET DurationIndex=9 WHERE id=28561;

-- Add SPELL_ATTR_EX3_DEATH_PERSISTENT to several non-passive spells which are triggered by items on equip
UPDATE spell_template SET AttributesEx3=AttributesEx3|0x00100000 WHERE Id IN(
'7363','16611','19307','17619','21079','22618','22620','23930','22988','23101','24198','28142','28143','28144','28145','28347','29112','29113',
'29162');

-- ============================================================
-- Classic section

-- Remove incorrect channel spell attribute for spell 29422 (Kel'Thuzad Channel Effect) used in Kel'Thuzad encounter
UPDATE spell_template SET AttributesEx=(AttributesEx&~0x00000040) WHERE id=29422;

-- Remove incorrect interrupt flag for Bellowing Roar spells used in various dragon encounters
UPDATE spell_template SET InterruptFlags=0 WHERE Id IN (18431, 22686);

UPDATE `spell_template` SET `EffectImplicitTargetA1` = 6 WHERE `Id` IN (25744,25787);

-- Cantation of Manifestation 9095 - insane radius 12 - 100y reduced to 13-10y
UPDATE spell_template SET EffectRadiusIndex1=13 WHERE id=9095;

-- Draco-Incarcinatrix 900
UPDATE spell_template SET EffectImplicitTargetA1=38 WHERE id=16007; -- 46

-- Flare - no delay is present here for stealth breaking - confirmed on classic
UPDATE spell_template SET Speed=0 WHERE id=1543; -- 5

-- Eye of Kilrogg - should not spawn 100 yards away from player
UPDATE spell_template SET EffectRadiusIndex1=15 WHERE Id IN(126);

-- Warrior - Execute damage spell - get rid of cost so normal handling does it without errors
UPDATE spell_template SET ManaCost=0 WHERE Id IN(20647);

-- Baron Rivendare 10440 does not enter combat and move while "channeling" Unholy Aura 17467
UPDATE `spell_template` SET `AttributesEx2`=AttributesEx2|0x00400000 WHERE `Id` = 17466; -- SPELL_ATTR_EX2_NO_INITIAL_AGGRO
UPDATE `spell_template` SET `AttributesEx` = `AttributesEx`&~0x00000004 WHERE `Id` = 17467; -- SPELL_ATTR_EX_CHANNELED_1

-- Remove SPELL_INTERRUPT_FLAG_ABORT_ON_DMG from Spells falsly interrupted by damage
UPDATE `spell_template` SET `InterruptFlags` = `InterruptFlags`&~0x00000010 WHERE `Id` IN (
13583, -- Curse of the Deadwood
26134, -- Eye Beam
28099, -- Shock
28440, -- Veil of Shadow
32950 -- Eye Beam
);

-- Fix periodic trigger pet buffing item auras giving threat - this fix is evidenced by 28757 which already has it
UPDATE spell_template SET AttributesEx=AttributesEx|1024 WHERE Id IN(21740,21921,21925,21927,20988,29233,27039,27042,27205,27208,18350,30023);

-- Wandering Plague (player version): as per parent spell tooltip should only targets friendly units, not every units nor self
-- SPELL_ATTR_EX_CANT_TARGET_SELF + TARGET_UNIT_ENEMY_NEAR_CASTER (parent spell caster is hostile to player and player's friends)
-- Note: could this be possibly handled through unimplemented SPELL_ATTR_EX_UNK11 ?
UPDATE spell_template SET `AttributesEx`=AttributesEx|0x00080000, EffectImplicitTargetA1=2, EffectImplicitTargetA2=2, EffectImplicitTargetA3=2 WHERE Id=3439;

-- Added Ignore LoS attribute for Web Wrab spells used in Maexxna encounter (trigger NPC is sometimes behind spider web GOs that break LoS with player)
UPDATE `spell_template` SET `AttributesEx2`=AttributesEx2|0x00000004 WHERE `id` BETWEEN 28617 AND 28621;

UPDATE spell_template SET Attributes=Attributes|0x04000000 WHERE Id IN(24378,23505,23493,24379,23451,23978); -- all battleground power-up spells: Berserking, Restoration, and Speed

-- LOS targeting section
-- All other confirmed totems that ignore LOS
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(8230,8250,8514,10607,10611,15036,25554,25579,25580);
-- Warrior Beneficial Shouts
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(469,2048,5242,6192,6673,11549,11550,11551,25289,27578);
-- Furious howl
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(24604,24605,24603,24597);
-- Unleashed Rage
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(30803,30804,30805,30806,30807);
-- Priest healing spells
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(596,996,10960,10961,25308,25316,23455,23458,23459,25329,27803,27804,27805);
-- Enchants
UPDATE spell_template SET AttributesEx2=AttributesEx2|0x00000004 WHERE Id IN(27996,28005);

-- BUG correction
-- equip only item spells with travel time
UPDATE spell_template SET Speed=0 WHERE Id IN(14824,14825,14826,14827,14828,14829,29413,29414,29415,29416,29417,29418,44972);

UPDATE `spell_template` SET `RangeIndex` = 1 WHERE `Id` IN (16613,16619,16630,16631);

-- Toxic Gas - used by Garden Gas in Naxx, remove SPELL_ATTR_EX_CHANNELED_1 and CHANNEL_FLAG_MOVEMENT to prevent aura being removed
UPDATE spell_template SET AttributesEx=0, ChannelInterruptFlags=0 WHERE Id=30074;

-- Lunar Festival fireworks spells have no missile speed serverside
UPDATE `spell_template` SET `speed`=0 WHERE `Id` IN (26286, 26291, 26292, 26294, 26295, 26333, 26334, 26336, 26337, 26338, 26304, 26325, 26327, 26328, 26329, 26488, 26490, 26517, 26519, 26518, 26521);

-- Gameobjects summoned by Lunar Festival fireworks only last one second
UPDATE `spell_template` SET `DurationIndex`=36 WHERE `Id` IN (26300, 26301, 26302, 26303, 26305, 26306, 26307, 26308, 26309, 26310, 26311, 26312, 26313, 26314, 26315, 26316, 26317, 26318, 26319, 26320, 26321, 26322, 26323, 26324, 26344, 26345, 26346, 26347, 26348, 26349, 26351, 26352, 26353, 26354, 26355, 26356, 26357, 26358, 26359, 26360, 26361, 26362, 26483, 26484, 26485, 26486, 26487, 26491, 26492, 26493, 26494, 26495, 26496, 26497, 26498, 26499, 26500, 26501, 26502, 26503, 26504, 26505, 26506, 26507, 26508, 26509, 26510, 26511, 26512, 26513, 26514, 26515);

-- ==================
-- MaxAffectedTargets
UPDATE `spell_template` SET `MaxAffectedTargets` = 1 WHERE `Id` IN (
802, -- Mutate Bug (AQ40, Emperor Vek'nilash)
804, -- Explode Bug (AQ40, Emperor Vek'lor)
23138, -- Gate of Shazzrah (MC, Shazzrah)
23173, -- Brood Affliction (BWL, Chromaggus) - TODO: Remove along with rework
23603, -- Wild Polymorph (BWL, Nefarian)
24019, -- Axe Flurry (ZG - Gurubashi Axe Thrower)
24150, -- Stinger Charge Primer (AQ20, Hive'Zara Stinger)
24781, -- Dream Fog (Emerald Dragons)
26080, -- Stinger Charge Primer (AQ40, Vekniss Stinger)
26237, -- AQ40 - C'Thun - Summon Mouth Tentacles - restricted to one target
26398, -- AQ40 - C'Thun - Summon Hook Tentacles - restricted to one target
26524, -- Sand Trap (AQ20 - Kurinnaxx)
28415, -- Summon Type A Trigger (Naxxramas, Kel'Thuzad)
28416, -- Summon Type B Trigger (Naxxramas, Kel'Thuzad)
28417, -- Summon Type C Trigger (Naxxramas, Kel'Thuzad)
28455, -- Summon Type D Trigger (Naxxramas, Kel'Thuzad)
28560 -- Summon Blizzard (Naxx, Sapphiron)
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 2 WHERE `Id` IN (
10258, -- Awaken Vault Warder (Uldaman)
28542 -- Life Drain (Naxx, Sapphiron)
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 5 WHERE `Id` IN (
29232 -- Fungal Bloom (Loatheb)
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 6 WHERE `Id` IN (
25676, -- Drain Mana (correct number has to be researched)
25754 -- Drain Mana
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 10 WHERE `Id` IN (
26180, -- AQ40 - Huhuran - Wyvern Sting/Poison Bolt Volley - Max 10 targets
28796, -- Poison Bolt Volley (Naxx, Faerlina)
29213 -- Curse of the Plaguebringer (Naxx, Noth the Plaguebringer)
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 12 WHERE `Id` IN (
26457, -- Drain Mana (correct number has to be researched)
26559 -- Drain Mana
);

UPDATE `spell_template` SET `MaxAffectedTargets` = 15 WHERE `Id` = 26052; -- Poison Bolt
-- MaxAffectedTargets
-- ==================

-- Ritual of summoning - LOS attribute - spell actually cast without target but needs GO type 18 rework
UPDATE `spell_template` SET `AttributesEx2`=AttributesEx2|4 WHERE `id` IN(7720);

-- Priest - Mind Vision Rank 2 should have unlimited range
UPDATE spell_template SET EffectRadiusIndex1=0 WHERE Id IN(10909);

-- Curse of Tongues - 1.12.0 - unexported to client but present in tbc - "This spell will no longer debuff the target if they are immune to the spellcast slowing effect."
UPDATE spell_template SET AttributesEx4=AttributesEx4|0x00000800 WHERE Id IN(1714,11719);

UPDATE `spell_template` SET `AttributesServerSide` = `AttributesServerSide`|4 WHERE `Id` IN (
3235, -- Rancid Blood
3338, -- Rancid Ooze
3418, -- Improved Blocking
5667, -- Bogling Passive
5888, -- Darkshore Frenzy
7131, -- Illusion Passive
8203, -- Elemental Spirit Invisibility
10092, -- Sand Storm
10868, -- Frost Vulnerable
11011, -- Stone Watcher of Norgannon Passive
11048, -- Perm. Illusion Bishop Tyriona
13377, -- Fire Shield
13886, -- Withering Poison Proc
15978, -- Puncture
21911, -- Puncture
24692, -- Hakkar Power
28330 -- Flameshocker - Immolate Visual
);

-- ============================================================
-- SPELL_ATTR_SS_IGNORE_EVADE - IsSpellRemovedOnEvade()
-- ============================================================
UPDATE `spell_template` SET `AttributesServerSide` = `AttributesServerSide`|0x00000004 WHERE `Id` IN (
4044,  -- Target Dummy Passive
4133,  -- Bruise
4161, -- Quick Snap
5301,  -- Defensive State (DND)
6742,  -- Bloodlust
7486, -- Butcher Drain
8273, -- Bonus Dodge
8393, -- Barbs
8434, -- Leech Pulse
8852,  -- Moss Hide
9463, -- Mirefin Fungus
9941, -- Spell Reflection
11816, -- Land Mine Arming
11966, -- Fire Shield
11984, -- Immolate
12002, -- Plague Cloud
12782, -- Shield Spike
12896, -- Helboar
12898, -- Smoke Aura Visual
13483, -- Wither Touch
13879, -- Magma Splash
16331, -- Incorporeal Defense
16577, -- Disease Cloud
16592, -- Shadowform
17205, -- Winterfall Firewater
17327, -- Spirit Particles
18115, -- Viewing Room Student Transform
18163, -- Strength of Arko'narin
18167, -- Holy Fire
19817, -- Double Attack
18951, -- Spirit Particles (green)
21080, -- Putrid Breath
21190, -- Aerie Gryphon Speed Buff
23378, -- Magma Splash
21130, -- Grip of Command
21788, -- Deadly Poison
21789, -- Hate to Half (Hate to Half)
27791, -- Suicide (Suicide)
28330, -- Flameshocker - Immolate Visual
30079 -- Retching Plague
);

-- Stop interrupt on damage taken for these 2 spells
UPDATE `spell_template` SET `InterruptFlags` = 15 WHERE `Id` in (8016,7901);

-- require backstab to face back
UPDATE spell_template SET AttributesServerside=AttributesServerside|0x00000008 WHERE Id IN(15582);
UPDATE `spell_template` SET AttributesServerside=AttributesServerside|0x00000008 WHERE `Id` IN (53,2589,2590,2591,6595,7159,8355,8721,11279,11280,11281,15582,15657,22416,24016,25300,26863); -- SPELL_ATTR_SS_FACING_BACK

-- Bonus coeff fixes
UPDATE spell_template SET EffectBonusCoefficient1=0.8 WHERE Id IN (1454,1455,1456,11687,11688,11689); -- Life Tap
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (1463,8494,8495,10191,10192,10193); -- Mana Shield
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (6229,11739,11740,28610); -- Shadow Ward
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (8188,10582,10583,10584); -- Magma Totem Passive
UPDATE spell_template SET EffectBonusCoefficient1=0.2142 WHERE Id IN (8443,8504,8505,11310,11311); -- Fire Nova Totem
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (8026,8028,8029,10445,16343,16344); -- Flametongue Weapon Proc
UPDATE spell_template SET EffectBonusCoefficient1=0.96 WHERE Id IN (18220,18937,18938); -- Dark Pact
UPDATE spell_template SET EffectBonusCoefficient1=1 WHERE Id IN (25997); -- Eye for an Eye
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (17,592,600,3747,6065,6066,10898,10899,10900,10901); -- Power Word: Shield
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (543,8457,8458,10223,10225); -- Fire Ward
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (6143,8461,8462,10177,28609); -- Frost Ward
UPDATE spell_template SET EffectBonusCoefficient1=0.1 WHERE Id IN (11426,13031,13032,13033); -- Ice Barrier
UPDATE spell_template SET EffectBonusCoefficient1=0.4485 WHERE Id IN (755,3698,3699,3700,11693,11694,11695); -- Health Funnel
UPDATE spell_template SET EffectBonusCoefficient1=0.33 WHERE Id IN (7001,27873,27874); -- Lightwell Renew

UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.03 WHERE Id IN (703,8631,8632,8633,11289,11290); -- Garrote
-- UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.2 WHERE Id IN (1495,14269,14270,14271); -- Mongoose Bite
UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.01, EffectBonusCoefficientFromAP2=0.02 WHERE Id IN (1822,1823,1824,9904); -- Rake
-- UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.02 WHERE Id IN (1978,13549,13550,13551,13552,13553,13554,13555,25295); -- Serpent Sting
-- UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.15 WHERE Id IN (3044,14281,14282,14283,14284,14285,14286,14287); -- Arcane Shot
UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.03 WHERE Id IN (9007,9824,9826); -- Pounce Bleed
-- UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.02 WHERE Id IN (13797,14298,14299,14300,14301); -- Immolation Trap Effect
-- UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.1, EffectBonusCoefficientFromAP2=0.1 WHERE Id IN (13812,14314,14315); -- Explosive Trap Effect
UPDATE spell_template SET EffectBonusCoefficientFromAP1=0.08 WHERE Id IN (779,780,769,9754,9908); -- Swipe
