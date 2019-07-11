
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
UPDATE spell_template SET Attributes=Attributes|0x00010000 WHERE Id IN(12328);

-- targeting section
-- Draw Spirit - Lethon - both effects need to have same radius index - faulty data
UPDATE spell_template SET EffectRadiusIndex1=EffectRadiusIndex2 WHERE Id IN(24811);
-- Opening Termite Barrel - target flags for GO/ITEM, effect targets GO/ITEM yet target targets units - fixes apparent typo
UPDATE spell_template SET EffectImplicitTargetA1=26 WHERE Id IN(18952);
-- Talents - Improved Scorch and Shadow Weaving have TARGET_UNIT_ENEMY, yet the auras are placed on caster anyway
-- It is possible talents are supposed to bypass normal casting, but evidenced by wotlk, the rest are all caster targeted, so assuming copypaste wrong data possibility as well
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(11095,12872,12873,15257,15331,15332,15333,15334);
-- Improved Wing Clip - item equipped aura which is meant to be cast on self
UPDATE spell_template SET EffectImplicitTargetA1=1 WHERE Id IN(19228,19232,19233,19234,19235);

-- Mangletooth - These spells need to ignore LOS
UPDATE spell_template SET AttributesEx2 = AttributesEx2|4 WHERE id IN (7764, 16618, 10767, 16610, 16612, 17013);

