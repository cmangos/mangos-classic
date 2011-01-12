ALTER TABLE db_version CHANGE COLUMN required_z1228_s0660_01_mangos_loot_template required_z1230_s0663_01_mangos_spell_bonus_data bit;

DELETE FROM spell_bonus_data WHERE entry = 172;
INSERT INTO spell_bonus_data  (entry, direct_bonus, dot_bonus, ap_bonus, comments) VALUES
(172,   0,      0.156,   0,     'Warlock - Corruption');
