ALTER TABLE db_version CHANGE COLUMN required_z1698_s1248_02_mangos_mangos_string required_z1719_s1272_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (539);

INSERT INTO mangos_string VALUES
(539,'Player selected: %s.\nFaction: %u.\nnpcFlags: %u.\nEntry: %u.\nDisplayID: %u (Native: %u).',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
