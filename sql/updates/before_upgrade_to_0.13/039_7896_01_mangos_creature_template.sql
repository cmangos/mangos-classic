ALTER TABLE db_version CHANGE COLUMN required_039_7896_01_mangos_creature_template required_035_7855_01_mangos_pools bit;

ALTER TABLE creature_template CHANGE COLUMN trainer_class class tinyint(3) unsigned NOT NULL default '0';
ALTER TABLE creature_template CHANGE COLUMN trainer_race race tinyint(3) unsigned NOT NULL default '0';
