DELETE FROM `command` WHERE `name` IN ('character reputation','pinfo');

INSERT INTO `command` VALUES
('pinfo',2,'Syntax: .pinfo [$player_name]\r\n\r\nOutput account information for selected player or player find by $player_name.'),
('character reputation',2,'Syntax: .character reputation [$player_name]\r\n\r\nShow reputation information for selected player or player find by $player_name.');
