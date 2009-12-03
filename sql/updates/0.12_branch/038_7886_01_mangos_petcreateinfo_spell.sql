DELETE FROM petcreateinfo_spell WHERE entry IN (SELECT entry FROM creature_template WHERE PetSpellDataId <> 0);
DELETE FROM petcreateinfo_spell WHERE Spell1=0 AND Spell2=0 AND Spell3=0 AND Spell4=0;
