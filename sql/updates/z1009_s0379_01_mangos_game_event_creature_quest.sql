ALTER TABLE db_version CHANGE COLUMN required_z0986_s0356_01_mangos_quest_template required_z1009_s0379_01_mangos_game_event_creature_quest bit;

ALTER TABLE game_event_creature_quest DROP PRIMARY KEY;
ALTER TABLE game_event_creature_quest ADD PRIMARY KEY (id,quest,event);
