ALTER TABLE db_version CHANGE COLUMN required_z2726_01_mangos_creature_conditional_spawn required_z2727_01_mangos_event_ai bit;

-- ALTER TABLE creature_ai_scripts DROP event_param5;
-- ALTER TABLE creature_ai_scripts DROP event_param6;
ALTER TABLE creature_ai_scripts ADD event_param5 int(11) NOT NULL DEFAULT '0' AFTER event_param4;
ALTER TABLE creature_ai_scripts ADD event_param6 int(11) NOT NULL DEFAULT '0' AFTER event_param5;

UPDATE `creature_ai_scripts` SET `action1_param2` = 12 WHERE `action1_param2` = 6 AND `action1_type` = 11 AND event_type IN(14,15,16);
UPDATE `creature_ai_scripts` SET `action2_param2` = 12 WHERE `action2_param2` = 6 AND `action2_type` = 11 AND event_type IN(14,15,16);
UPDATE `creature_ai_scripts` SET `action3_param2` = 12 WHERE `action3_param2` = 6 AND `action3_type` = 11 AND event_type IN(14,15,16);

UPDATE creature_ai_scripts SET event_param3=1 WHERE event_type=5;