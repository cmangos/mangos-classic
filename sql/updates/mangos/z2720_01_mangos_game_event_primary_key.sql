ALTER TABLE db_version CHANGE COLUMN required_z2719_01_mangos_taxi_system_update required_z2720_01_mangos_game_event_primary_key bit;

ALTER TABLE game_event_gameobject DROP PRIMARY KEY, ADD PRIMARY KEY (guid, event);
ALTER TABLE game_event_creature DROP PRIMARY KEY, ADD PRIMARY KEY (guid, event);

