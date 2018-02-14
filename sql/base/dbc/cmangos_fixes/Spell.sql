-- Spell 24148 should have passive attribute. Without it, items with that spell enchant are declared as invalid when logging in a dead player
-- Confirmed on later version of the spell
UPDATE spell_template SET Attributes = Attributes | 0x040 WHERE Id = 24148;

-- Removes aura interrupt flag by sitting for spells that do not make a creature sit, all seem to be copypasted for unknown reasons
UPDATE spell_template SET AuraInterruptFlags=AuraInterruptFlags&~0x00040000 WHERE Id IN(6606,14915,16093);

UPDATE spell_template SET EffectItemType2=268435456 WHERE Id IN(16106); -- all ranks except rank 3 have mask
UPDATE spell_template SET EffectItemType1=0x00240000 WHERE Id IN(12842); -- all ranks except rank 5 have mask

-- Allow caster to cast this self-target spell even if he is immune to the spell damage school
UPDATE spell_template SET Attributes = Attributes | 0x20000000 WHERE id IN (22972, 22975, 22976, 22977, 22978, 22979, 22980, 22981, 22982, 22983, 22984, 22985, 22986);

-- Fixed radius for the various spells used in summoning NPCs in Nefarian encounter
-- Value in DBC made the adds spawn anywhere instead of near their spawner NPC
UPDATE spell_template SET EffectRadiusIndex1=7 WHERE id IN (22654, 22655, 22656, 22657, 22658, 22680);

-- Fixes spell 13166 (Gnomish Battle Chicken): the summoning type was wrong
UPDATE spell_template SET Effect1=42, EffectBasePoints1=0 WHERE id=13166;

