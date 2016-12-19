ALTER TABLE db_version CHANGE COLUMN required_z2691_01_mangos_spell_template required_z2692_01_mangos_areatrigger_teleport bit;

ALTER TABLE areatrigger_teleport ADD COLUMN `condition_id` INT(11) unsigned NOT NULL default '0' AFTER target_orientation;
