DELETE FROM `command` WHERE `name` IN ('gobject near');

INSERT INTO `command` VALUES
('gobject near',2,'Syntax: .gobject near  [#distance]\r\n\r\nOutput gameobjects at distance #distance from player. Output gameobject guids and coordinates sorted by distance from character. If #distance not provided use 10 as default value.');
