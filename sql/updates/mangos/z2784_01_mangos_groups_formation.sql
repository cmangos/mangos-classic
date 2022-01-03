ALTER TABLE db_version CHANGE COLUMN required_z2783_01_mangos_spawn_groups required_z2784_01_mangos_groups_formation bit;

ALTER TABLE `spawn_group_spawn` ADD COLUMN `SlotId` tinyint(4) NOT NULL DEFAULT -1 COMMENT '0 is the leader, -1 not part of the formation' AFTER `Guid`;

DROP TABLE IF EXISTS `spawn_group_formation`;
CREATE TABLE `spawn_group_formation`  (
  `SpawnGroupID` int(11) NOT NULL COMMENT 'Spawn group id',
  `FormationType` tinyint(11) NOT NULL DEFAULT 0 COMMENT 'Formation shape 0..6',
  `FormationSpread` float(11, 0) NOT NULL DEFAULT 0 COMMENT 'Distance between formation members',
  `FormationOptions` int(11) NOT NULL DEFAULT 0 COMMENT 'Keep formation compact (bit 1)',
  `MovementID` int(11) NOT NULL DEFAULT 0 COMMENT 'Id from waypoint_path path',
  `MovementType` tinyint(11) NOT NULL COMMENT 'Same as creature table',
  `Comment` varchar(255) NULL DEFAULT NULL,
  PRIMARY KEY (`SpawnGroupID`)
);

DROP TABLE IF EXISTS `waypoint_path`;
CREATE TABLE `waypoint_path`  (
  `entry` mediumint(8) UNSIGNED NOT NULL COMMENT 'Creature entry',
  `pathId` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Path ID for entry',
  `point` mediumint(8) UNSIGNED NOT NULL DEFAULT 0,
  `position_x` float NOT NULL DEFAULT 0,
  `position_y` float NOT NULL DEFAULT 0,
  `position_z` float NOT NULL DEFAULT 0,
  `orientation` float NOT NULL DEFAULT 0,
  `waittime` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `script_id` mediumint(8) UNSIGNED NOT NULL DEFAULT 0,
  `comment` text NULL DEFAULT NULL,
  PRIMARY KEY (`entry`, `pathId`, `point`)
);
