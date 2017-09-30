-- Spell 24148 should have passive attribute. Without it, items with that spell enchant are declared as invalid when logging in a dead player
-- Confirmed on later version of the spell
UPDATE spell_template SET Attributes = Attributes | 0x040 WHERE Id = 24148;

-- Removes aura interrupt flag by sitting for spells that do not make a creature sit, all seem to be copypasted for unknown reasons
UPDATE spell_template SET AuraInterruptFlags=AuraInterruptFlags&~0x00040000 WHERE Id IN(6606,14915,16093);

UPDATE spell_template SET EffectItemType2=268435456 WHERE Id IN(16106); -- all ranks except rank 3 have mask
UPDATE spell_template SET EffectItemType1=0x00240000 WHERE Id IN(12842); -- all ranks except rank 5 have mask

