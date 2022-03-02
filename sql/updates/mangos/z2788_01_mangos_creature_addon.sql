ALTER TABLE db_version CHANGE COLUMN required_z2787_01_mangos_npc_vendor required_z2788_01_mangos_creature_addon bit;

ALTER TABLE `creature_addon` ADD COLUMN `interaction_pause_timer` INT(10) DEFAULT -1 NOT NULL AFTER `moveflags`;
ALTER TABLE `creature_template_addon` ADD COLUMN `interaction_pause_timer` INT(10) DEFAULT -1 NOT NULL AFTER `moveflags`;

