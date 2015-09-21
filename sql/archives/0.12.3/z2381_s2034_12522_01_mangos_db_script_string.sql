ALTER TABLE db_version CHANGE COLUMN required_z2319_s1960_12440_01_mangos_spell_area required_z2381_s2034_12522_01_mangos_db_script_string bit;


ALTER TABLE db_script_string ADD COLUMN sound mediumint(8) unsigned NOT NULL DEFAULT '0' AFTER content_loc8;
ALTER TABLE db_script_string ADD COLUMN type tinyint(3) unsigned NOT NULL DEFAULT '0' AFTER sound;
ALTER TABLE db_script_string ADD COLUMN language tinyint(3) unsigned NOT NULL DEFAULT '0' AFTER type;
ALTER TABLE db_script_string ADD COLUMN emote smallint(5) unsigned NOT NULL DEFAULT '0' AFTER language;
ALTER TABLE db_script_string ADD COLUMN comment text AFTER emote;

-- Update Dbscript_string with the type and language from Dbscripts_on_*
CREATE TABLE IF NOT EXISTS db_script_temp AS
-- dbscripts_on_creature_death
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_creature_death B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_creature_movement
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_creature_movement B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_event
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_event B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_gossip
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_gossip B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_go_template_use
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_go_template_use B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_go_use
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_go_use B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_quest_end
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_quest_end B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_quest_start
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_quest_start B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4)
-- dbscripts_on_spell
UNION ALL
SELECT A.entry
    ,A.content_default
    ,A.content_loc1
    ,A.content_loc2
    ,A.content_loc3
    ,A.content_loc4
    ,A.content_loc5
    ,A.content_loc6
    ,A.content_loc7
    ,A.content_loc8
    ,0 AS sound
    ,B.datalong AS type
    ,B.datalong2 AS language
    ,0 AS emote
    ,NULL AS comment
FROM db_script_string A
INNER JOIN dbscripts_on_spell B ON (A.entry = B.dataint OR A.entry = B.dataint2 OR A.entry = B.dataint3 OR A.entry = B.dataint4);

-- Clean dbscript_string and insert the new updated values
DELETE FROM db_script_string WHERE entry IN (SELECT DISTINCT entry FROM db_script_temp);
INSERT INTO db_script_string SELECT * FROM db_script_temp GROUP BY entry;
DROP TABLE IF EXISTS db_script_temp;
