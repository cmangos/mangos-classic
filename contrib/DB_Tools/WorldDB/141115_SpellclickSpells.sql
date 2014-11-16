-- Fields in npc_spellclick_spells table that should be removed

-- -- The columns quest_start, quest_start_active, quest_end
-- --   of table npc_spellclick_spells will be removed

-- -- Output limited to 10 lines.

-- Related commit: e91ad7 - [12441] Add condition_id support to npc_spellclick_spells
SELECT * FROM npc_spellclick_spells WHERE quest_start !=0 OR quest_start_active !=0 OR quest_end !=0 LIMIT 10;
