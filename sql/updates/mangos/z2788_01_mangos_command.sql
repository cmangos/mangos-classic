DELETE FROM `command` WHERE `name`="debug areatriggers";
INSERT INTO `command` (`name`, `security`, `help`) VALUES
("debug areatriggers", 1, "Syntax: .debug areatriggers\n\nToggle debug mode for areatriggers. In debug mode GM will be notified if reaching an areatrigger.");

DELETE FROM `mangos_string` WHERE `entry` IN (183, 184, 185);
INSERT INTO `mangos_string` (`entry`, `content_default`) VALUES
(183, "Areatrigger debugging turned on."),
(184, "Areatrigger debugging turned off."),
(185, "You have reached areatrigger %u.");

