ALTER TABLE db_version CHANGE COLUMN required_z2530_sxxxx_xxxxx_01_mangos_gameobject_template required_z2533_sxxxx_xxxxx_01_mangos_spell_chain bit;

DELETE FROM spell_chain WHERE spell_id IN (12966, 12967, 12968, 12969, 12970, 16257, 16277, 16278, 16279, 16280);
INSERT INTO spell_chain VALUES
/* Flurry triggered, Warrior */
(12966,0,12966,1,0),
(12967,12966,12966,2,0),
(12968,12967,12966,3,0),
(12969,12968,12966,4,0),
(12970,12969,12966,5,0),
/* Flurry triggered, Shaman */
(16257,0,16257,1,0),
(16277,16257,16257,2,0),
(16278,16277,16257,3,0),
(16279,16278,16257,4,0),
(16280,16279,16257,5,0);
