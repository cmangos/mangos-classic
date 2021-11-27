ALTER TABLE db_version CHANGE COLUMN required_z2780_01_mangos_creature_spell_lists required_z2781_01_mangos_dbscripts bit;

ALTER TABLE dbscripts_on_creature_movement CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_event CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_go_use CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_go_template_use CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_gossip CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_quest_end CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_quest_start CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE dbscripts_on_creature_death CHANGE COLUMN datalong datalong INT(10) UNSIGNED NOT NULL DEFAULT '0';
