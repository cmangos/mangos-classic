DELETE FROM `command` WHERE `name`="list areatriggers";
INSERT INTO `command` (`name`, `security`, `help`) VALUES
("list areatriggers", 3, "Syntax: .list areatriggers\n\nShow areatriggers within the same map (if inside an instanceable map) or area (if inside a continent) as the user.");

DELETE FROM `mangos_string` WHERE `entry`=555;
INSERT INTO `mangos_string` (`entry`, `content_default`) VALUES
(555, "Showing all areatriggers in %s %s:");

