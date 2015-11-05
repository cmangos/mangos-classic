ALTER TABLE db_version CHANGE COLUMN required_z1192_s0612_02_mangos_spell_chain required_z1194_s0611_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (1166,1167,1168);

INSERT INTO mangos_string VALUES
(1166,'Scripting library not found or not accessable.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1167,'Scripting library has wrong list functions (outdated?).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(1168,'Scripting library reloaded.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
