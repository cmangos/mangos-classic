ALTER TABLE db_version CHANGE COLUMN required_z2746_01_mangos_world_safe_locs_facing required_z2747_01_mangos_broadcast_text bit;

DROP TABLE IF EXISTS `npc_text_broadcast_text`;
CREATE TABLE `npc_text_broadcast_text` (
`Id` MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'Identifier',
`Prob0` float NOT NULL,
`Prob1` float NOT NULL,
`Prob2` float NOT NULL,
`Prob3` float NOT NULL,
`Prob4` float NOT NULL,
`Prob5` float NOT NULL,
`Prob6` float NOT NULL,
`Prob7` float NOT NULL,
`BroadcastTextId1` INT(11) NOT NULL,
`BroadcastTextId2` INT(11) NOT NULL,
`BroadcastTextId3` INT(11) NOT NULL,
`BroadcastTextId4` INT(11) NOT NULL,
`BroadcastTextId5` INT(11) NOT NULL,
`BroadcastTextId6` INT(11) NOT NULL,
`BroadcastTextId7` INT(11) NOT NULL,
PRIMARY KEY(`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT 'Broadcast Text npc_text equivalent';

ALTER TABLE gossip_menu_option ADD COLUMN option_broadcast_text INT(11) NOT NULL DEFAULT '0' AFTER option_text;
ALTER TABLE gossip_menu_option ADD COLUMN box_broadcast_text INT(11) NOT NULL DEFAULT '0' AFTER box_text;

ALTER TABLE script_texts ADD COLUMN broadcast_text_id INT(11) NOT NULL DEFAULT '0' AFTER emote;
ALTER TABLE creature_ai_texts ADD COLUMN broadcast_text_id INT(11) NOT NULL DEFAULT '0' AFTER emote;
ALTER TABLE dbscript_string ADD COLUMN broadcast_text_id INT(11) NOT NULL DEFAULT '0' AFTER emote;
ALTER TABLE custom_texts ADD COLUMN broadcast_text_id INT(11) NOT NULL DEFAULT '0' AFTER emote;

