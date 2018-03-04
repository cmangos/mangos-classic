ALTER TABLE db_version CHANGE COLUMN required_z2718_01_mangos_spell_affect required_z2719_01_mangos_creature_template_fixup bit;

ALTER TABLE `creature_template` CHANGE COLUMN FactionAlliance Faction smallint(5) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_template` DROP COLUMN FactionHorde;
