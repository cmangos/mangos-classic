-- Spell 24148 should have passive attribute. Without it, items with that spell enchant are declared as invalid when logging in a dead player
-- Confirmed on later version of the spell
UPDATE spell_template SET Attributes = Attributes | 0x040 WHERE Id = 24148;
