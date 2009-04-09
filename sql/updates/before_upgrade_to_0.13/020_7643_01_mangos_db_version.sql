ALTER TABLE db_version CHANGE COLUMN required_020_7643_01_mangos_db_version required_2008_12_15_01_mangos_arenas bit;

ALTER TABLE db_version
   DROP COLUMN creature_ai_version;
