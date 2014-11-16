-- Fields in spell_area table that should be removed

-- -- The columns quest_start, quest_start_active, quest_end, racemask, gender
-- --   of table spell_are will be removed

-- -- Output limited to 10 lines.

-- Related commit: ef5e75 - [12440] Add support for condition_id to spell_area table
SELECT * FROM spell_area WHERE quest_start !=0 OR quest_start_active !=0 OR quest_end !=0 OR racemask !=0 OR gender !=0 LIMIT 10;
