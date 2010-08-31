ALTER TABLE db_version CHANGE COLUMN required_z0817_xxxxx_01_mangos_game_event required_z0830_s0148_01_mangos_spell_chain bit;

DELETE FROM spell_chain WHERE first_spell IN (
  25076, 24398, 688, 6542, 27683, 26064, 469, 603, 974,
  1329, 1804, 2062, 2651, 2825, 2894, 3738, 5277, 5938,
  23028, 20164, 20243, 24224
);

INSERT INTO spell_chain VALUES
/*Elune's Grace*/
(2651,0,2651,1,0),
(19289,2651,2651,2,0),
(19291,19289,2651,3,0),
(19292,19291,2651,4,0),
(19293,19292,2651,5,0);
