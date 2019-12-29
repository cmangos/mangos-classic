ALTER TABLE db_version CHANGE COLUMN required_z2747_01_mangos_broadcast_text required_z2749_01_mangos_dbscript_priority_miliseconds bit;

ALTER TABLE dbscripts_on_creature_death ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_creature_movement ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_event ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_go_template_use ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_go_use ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_gossip ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_quest_end ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_quest_start ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_spell ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;
ALTER TABLE dbscripts_on_relay ADD COLUMN priority INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER delay;

UPDATE dbscripts_on_creature_death SET delay=delay*1000;
UPDATE dbscripts_on_creature_movement SET delay=delay*1000;
UPDATE dbscripts_on_event SET delay=delay*1000;
UPDATE dbscripts_on_go_template_use SET delay=delay*1000;
UPDATE dbscripts_on_go_use SET delay=delay*1000;
UPDATE dbscripts_on_gossip SET delay=delay*1000;
UPDATE dbscripts_on_quest_end SET delay=delay*1000;
UPDATE dbscripts_on_quest_start SET delay=delay*1000;
UPDATE dbscripts_on_spell SET delay=delay*1000;
UPDATE dbscripts_on_relay SET delay=delay*1000;

