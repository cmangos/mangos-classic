ALTER TABLE db_version CHANGE COLUMN required_z2720_01_mangos_game_event_primary_key required_z2721_01_mangos_creature_template_spells_extension bit;

ALTER TABLE creature_template_spells ADD COLUMN `spell5` mediumint(8) unsigned NOT NULL DEFAULT '0';
ALTER TABLE creature_template_spells ADD COLUMN `spell6` mediumint(8) unsigned NOT NULL DEFAULT '0';
ALTER TABLE creature_template_spells ADD COLUMN `spell7` mediumint(8) unsigned NOT NULL DEFAULT '0';
ALTER TABLE creature_template_spells ADD COLUMN `spell8` mediumint(8) unsigned NOT NULL DEFAULT '0';


