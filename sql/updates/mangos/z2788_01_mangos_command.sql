DELETE FROM `command` WHERE `name`="tele warp";
INSERT INTO `command` (`name`, `security`, `help`) VALUES
("tele warp", 1, "Syntax: .tele warp #axis #value\n\nTeleports the user by the specified value along the specified axis.\nUse a positive value to move forward the axis, and a negative value to move backward the axis.\nValid axis are x (+forward/-backward), y (+right/-left), z (+above/-below), o (orientation, value is specified in degrees).");

