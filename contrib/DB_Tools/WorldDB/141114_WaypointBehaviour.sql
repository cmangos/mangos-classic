-- Waypoint-Movement: behaviour fields should be removed

-- -- The columns textidX, emote, spell, modelY of tables creature_movement[_template] should be dropped
-- -- Script_id added may 2010 and can handle all the behavior.
-- -- Output limited to 20 lines.

-- Related commit: 1d577a - [9891] Implement *_scripts for creature_movement
SELECT * FROM creature_movement WHERE textid1 !=0 OR textid2 !=0 OR textid3 !=0 OR textid4 !=0 OR textid5 !=0 OR emote !=0 OR spell !=0 OR model1 !=0 OR model2 !=0 LIMIT 10;
SELECT * FROM creature_movement_template WHERE textid1 !=0 OR textid2 !=0 OR textid3 !=0 OR textid4 !=0 OR textid5 !=0 OR emote !=0 OR spell !=0 OR model1 !=0 OR model2 !=0 LIMIT 10;
