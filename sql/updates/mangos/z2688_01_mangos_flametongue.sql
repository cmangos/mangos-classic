ALTER TABLE db_version CHANGE COLUMN required_z2687_01_mangos_spell_chain required_z2688_01_mangos_flametongue bit;

DELETE FROM spell_bonus_data WHERE entry IN (8026, 8028, 8029, 10445, 16343, 16344);
INSERT INTO spell_bonus_data VALUES('8026', '0.1', '0', '0', '0', 'Shaman - Flametongue Weapon Proc Rank 1');

DELETE FROM spell_chain WHERE first_spell=8026;
INSERT INTO spell_chain VALUES
(8026, 0, 8026, 1, 0),
(8028, 8026, 8026, 2, 0),
(8029, 8028, 8026, 3, 0),
(10445, 8029, 8026, 4, 0),
(16343, 10445, 8026, 5, 0),
(16344, 16343, 8026, 6, 0);

