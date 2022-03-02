ALTER TABLE db_version CHANGE COLUMN required_z2787_01_mangos_npc_vendor required_z2788_01_mangos_creature_addon bit;

ALTER TABLE `creature_template` ADD COLUMN `InteractionPauseTimer` INT(10) DEFAULT -1 NOT NULL AFTER `GossipMenuId`;

