ALTER TABLE db_version CHANGE COLUMN required_z2778_02_mangos_gameobject_spawn_entry required_z2779_01_mangos_dbscript_data_flags_widening bit;

ALTER TABLE dbscripts_on_creature_death MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_creature_movement MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_event MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_gossip MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_go_template_use MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_go_use MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_quest_end MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_quest_start MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_relay MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_spell MODIFY data_flags INT UNSIGNED NOT NULL DEFAULT '0';

