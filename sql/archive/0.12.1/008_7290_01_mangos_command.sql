DELETE FROM `command` WHERE `name` = 'npc setdeathstate';
INSERT INTO `command` VALUES
('npc setdeathstate',2,'Syntax: .npc setdeathstate on/off\r\n\r\nSet default death state (dead/alive) for npc at spawn.');
