ALTER TABLE db_version CHANGE COLUMN required_z1217_s0636_03_mangos_game_event_mail required_z1218_s0638_01_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE `entry` = 18790;
INSERT INTO spell_bonus_data (entry, direct_bonus, dot_bonus, ap_bonus, comments) VALUES
(18790, 0, 0, 0,'Warlock - Fel Stamina');