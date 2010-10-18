ALTER TABLE db_version CHANGE COLUMN required_z0919_s0298_02_mangos_spell_chain required_z0967_s0263_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry IN (726,750,751);
INSERT INTO mangos_string VALUES (750,'Not enough players. This game will close in %u mins.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
INSERT INTO mangos_string VALUES (751,'Not enough players. This game will close in %u seconds.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
