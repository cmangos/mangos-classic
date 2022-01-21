ALTER TABLE db_version CHANGE COLUMN required_z2784_01_mangos_groups_formation required_z2785_01_mangos_waypoint_path bit;

ALTER TABLE `spawn_group_formation` CHANGE `SpawnGroupID` `Id` INT NOT NULL;
ALTER TABLE `spawn_group_formation` CHANGE `MovementID` `PathId` INT NOT NULL;

UPDATE `waypoint_path` SET `pathId` = `entry`;

ALTER TABLE `waypoint_path` CHANGE `pathId` `PathId` INT UNSIGNED NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `point` `Point` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `position_x` `PositionX` FLOAT NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `position_y` `PositionY` FLOAT NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `position_z` `PositionZ` FLOAT NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `orientation` `Orientation` FLOAT NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `waittime` `WaitTime` INT UNSIGNED NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `script_id` `ScriptId` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `waypoint_path` CHANGE `comment` `Comment` TEXT NULL;
ALTER TABLE `waypoint_path` DROP PRIMARY KEY;
ALTER TABLE `waypoint_path` ADD PRIMARY KEY (`PathId`, `Point`);
ALTER TABLE `waypoint_path` DROP COLUMN `entry`;

ALTER TABLE `creature_movement` CHANGE `id` `Id` INT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement` CHANGE `point` `Point` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement` CHANGE `position_x` `PositionX` FLOAT NOT NULL;
ALTER TABLE `creature_movement` CHANGE `position_y` `PositionY` FLOAT NOT NULL;
ALTER TABLE `creature_movement` CHANGE `position_z` `PositionZ` FLOAT NOT NULL;
ALTER TABLE `creature_movement` CHANGE `orientation` `Orientation` FLOAT NOT NULL;
ALTER TABLE `creature_movement` CHANGE `waittime` `WaitTime` INT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement` CHANGE `script_id` `ScriptId` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement` CHANGE `comment` `Comment` TEXT NULL;

ALTER TABLE `creature_movement_template` CHANGE `entry` `Entry` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `pathId` `PathId` INT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `point` `Point` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `position_x` `PositionX` FLOAT NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `position_y` `PositionY` FLOAT NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `position_z` `PositionZ` FLOAT NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `orientation` `Orientation` FLOAT NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `waittime` `WaitTime` INT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `script_id` `ScriptId` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `creature_movement_template` CHANGE `comment` `Comment` TEXT NULL;

ALTER TABLE `script_waypoint` CHANGE `entry` `Entry` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `pathId` `PathId` INT UNSIGNED NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `pointid` `Point` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `position_x` `PositionX` FLOAT NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `position_y` `PositionY` FLOAT NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `position_z` `PositionZ` FLOAT NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `orientation` `Orientation` FLOAT NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `waittime` `WaitTime` INT UNSIGNED NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `script_id` `ScriptId` MEDIUMINT UNSIGNED NOT NULL;
ALTER TABLE `script_waypoint` CHANGE `comment` `Comment` TEXT NULL;

