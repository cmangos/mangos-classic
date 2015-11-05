ALTER TABLE db_version CHANGE COLUMN required_z1227_s0658_01_mangos_spell_bonus_data required_z1228_s0660_01_mangos_loot_template bit;

ALTER TABLE creature_loot_template
  CHANGE COLUMN entry entry mediumint(8) unsigned NOT NULL default '0' COMMENT 'entry 0 used for player insignia loot';

ALTER TABLE fishing_loot_template
  CHANGE COLUMN entry entry mediumint(8) unsigned NOT NULL default '0' COMMENT 'entry 0 used for junk loot at fishing fail (if allowed by config option)';
