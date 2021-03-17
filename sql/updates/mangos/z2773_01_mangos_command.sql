ALTER TABLE db_version CHANGE COLUMN required_z2772_01_mangos_dbscript_npc_flag_update required_z2773_01_mangos_command bit;

DELETE FROM command WHERE name IN ('combatstop', 'combat stop', 'gm', 'gm visible', 'gm chat', 'character deleted list', 'gm setview', 'movement movespeed', 'movement movegens', 'gm ingame', 'gm mountup');

INSERT INTO `command`(`name`, `security`, `help`) VALUES
('combat stop', 2, 'Syntax: .combatstop [$playername]\r\nStop combat for selected character. If selected non-player then command applied to self. If $playername provided then attempt applied to online player $playername.'),
('gm', 3, 'Syntax: .gm [on/off]\r\n\r\nEnable or Disable in game GM MODE or show current state of on/off not provided.'),
('gm visible', 3, 'Syntax: .gm visible on/off\r\n\r\nOutput current visibility state or make GM visible(on) and invisible(off) for other players.'),
('gm chat', 3, 'Syntax: .gm chat [on/off]\r\n\r\nEnable or disable chat GM MODE (show gm badge in messages) or show current state of on/off not provided.'),
('character deleted list', 4, 'Syntax: .character deleted list [#guid|$name]\r\n\r\nShows a list with all deleted characters.\r\nIf $name is supplied, only characters with that string in their name will be selected, if #guid is supplied, only the character with that GUID will be selected.'),
('gm setview', 3, 'Syntax: .gm setview\r\n\r\nSet farsight view on selected unit. Select yourself to set view back.'),
('movement movespeed', 2, 'Syntax: .movement movespeed  Show speed of selected creature.'),
('movement movegens', 2, 'Syntax: .movement movegens  Show movement generators stack for selected creature or player.'),
('gm ingame', 3, 'Syntax: .gm ingame\r\n\r\nDisplay a list of available in game Game Masters.'),
('gm mountup', 3, 'Syntax: .gm mountup [fast|slow|#displayid|target]\r\n\r\nIf #displayid is provided, visually mounts your character on a provided creature likeness. If your target is a creature and corresponding arg is provided, visually mounts your character on the likeness of targeted creature. In other cases this command mounts your character on a random unusual land mount according to provided or omitted args.');

