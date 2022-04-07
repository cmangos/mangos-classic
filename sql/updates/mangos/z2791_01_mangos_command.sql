ALTER TABLE db_version CHANGE COLUMN required_z2790_01_mangos_quest_template required_z2791_01_mangos_command bit;

DELETE FROM command WHERE name = 'list object';

REPLACE INTO `command` VALUES
('list object', 3, 'Syntax: .list object #gameobject_id [#max_count]\r\n.list object #gameobject_id [world|zone|area|map] [#max_count]\r\nOutput gameobjects with gameobject id #gameobject_id found in world. Output gameobject guids and coordinates sorted by distance from character. Will be output maximum #max_count gameobject. If #max_count not provided use 200 as default value.');
