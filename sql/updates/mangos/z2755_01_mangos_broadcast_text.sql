ALTER TABLE db_version CHANGE COLUMN required_z2754_01_mangos_static_custom_chat required_z2755_01_mangos_broadcast_text bit;

ALTER TABLE `npc_text_broadcast_text` MODIFY Prob1 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob2 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob3 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob4 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob5 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob6 float NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY Prob7 float NOT NULL DEFAULT '0';

ALTER TABLE `npc_text_broadcast_text` ADD COLUMN `BroadcastTextId0` INT(11) NOT NULL AFTER `Prob7`;
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId1` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId3` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId4` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId5` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId6` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `npc_text_broadcast_text` MODIFY `BroadcastTextId7` INT(11) NOT NULL DEFAULT '0';


