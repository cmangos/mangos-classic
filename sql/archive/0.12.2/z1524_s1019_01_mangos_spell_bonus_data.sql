ALTER TABLE db_version CHANGE COLUMN required_z1488_s0974_01_mangos_mangos_string required_z1524_s1019_01_mangos_spell_bonus_data bit;

ALTER TABLE spell_bonus_data
  ADD COLUMN ap_dot_bonus float NOT NULL default '0' AFTER ap_bonus;


DELETE FROM spell_bonus_data WHERE entry in (26573,703);
INSERT INTO `spell_bonus_data` VALUES
(26573, 0,      0.04,    0,  0.04,     'Paladin - Consecration'),
(703,   0,      0,       0,     0.03,  'Rogue - Garrote');