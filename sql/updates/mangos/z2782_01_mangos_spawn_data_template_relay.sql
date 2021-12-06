ALTER TABLE db_version CHANGE COLUMN required_z2781_01_mangos_dbscripts required_z2782_01_mangos_spawn_data_template_relay bit;

ALTER TABLE creature_spawn_data_template ADD COLUMN RelayId INT UNSIGNED NOT NULL COMMENT 'dbscripts_on_relay' AFTER SpawnFlags;

