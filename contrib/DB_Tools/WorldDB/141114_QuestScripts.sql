-- Find Quest Start/End Scripts that are unused

-- -- These queries report entries in dbscripts_on_quest_start/ dbscripts_on_quest_end
-- --   that are unused from quest_template. StartScript/ CompleteScript

-- Catch unused start scripts
SELECT b.id FROM dbscripts_on_quest_start AS b WHERE b.id NOT IN (SELECT StartScript FROM quest_template WHERE StartScript!=0);
-- Catch unused end scripts
SELECT b.id FROM dbscripts_on_quest_end AS b WHERE b.id NOT IN (SELECT CompleteScript FROM quest_template WHERE CompleteScript!=0);
