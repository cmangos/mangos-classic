ALTER TABLE db_version CHANGE COLUMN required_z2794_01_mangos_spawn_group_chanced_spawns required_z2795_01_mangos_waypoint_path_name bit;

DROP TABLE IF EXISTS waypoint_path_name;
CREATE TABLE waypoint_path_name(
  `PathId` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Path ID for entry',
  `Name` VARCHAR(300) NOT NULL COMMENT 'Description of usage',
  PRIMARY KEY(`PathId`)
);


