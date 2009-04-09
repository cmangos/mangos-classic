ALTER TABLE db_version CHANGE COLUMN required_2008_12_15_01_mangos_arenas required_020_7643_01_mangos_db_version bit;

ALTER TABLE db_version
   ADD COLUMN creature_ai_version varchar(120) default NULL AFTER version;

UPDATE db_version
  SET creature_ai_version = 'Mangos default creature EventAI data.';
