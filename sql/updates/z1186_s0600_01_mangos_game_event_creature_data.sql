ALTER TABLE db_version CHANGE COLUMN required_z1159_s0566_01_mangos_mangos_string required_z1186_s0600_01_mangos_game_event_creature_data bit;


DROP TABLE IF EXISTS game_event_creature_data;
RENAME TABLE game_event_model_equip TO game_event_creature_data;

ALTER TABLE game_event_creature_data
  ADD COLUMN entry_id mediumint(8) unsigned NOT NULL default '0' AFTER guid,
  ADD COLUMN spell_start mediumint(8) unsigned NOT NULL default '0' AFTER equipment_id,
  ADD COLUMN spell_end mediumint(8) unsigned NOT NULL default '0' AFTER spell_start;

ALTER TABLE game_event_creature_data
  DROP PRIMARY KEY,
  ADD PRIMARY KEY  (`guid`,`event`);
