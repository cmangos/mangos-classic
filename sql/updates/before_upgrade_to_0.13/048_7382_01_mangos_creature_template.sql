ALTER TABLE db_version CHANGE COLUMN required_048_7382_01_mangos_creature_template required_045_8098_02_mangos_playercreateinfo_action bit;

ALTER TABLE creature_template
  DROP COLUMN unk16,
  DROP COLUMN unk17;
