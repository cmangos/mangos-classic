ALTER TABLE db_version CHANGE COLUMN required_z2784_01_mangos_groups_formation required_z2785_01_mangos_waypoint_path bit;

ALTER TABLE `spawn_group_formation` RENAME COLUMN `SpawnGroupID` TO `Id`;
ALTER TABLE `spawn_group_formation` RENAME COLUMN `MovementID` TO `PathId`;

UPDATE `waypoint_path` SET `pathId` = `entry`;

ALTER TABLE `waypoint_path` RENAME COLUMN `pathId` TO `PathId`;
ALTER TABLE `waypoint_path` RENAME COLUMN `point` TO `Point`;
ALTER TABLE `waypoint_path` RENAME COLUMN `position_x` TO `PositionX`;
ALTER TABLE `waypoint_path` RENAME COLUMN `position_y` TO `PositionY`;
ALTER TABLE `waypoint_path` RENAME COLUMN `position_z` TO `PositionZ`;
ALTER TABLE `waypoint_path` RENAME COLUMN `orientation` TO `Orientation`;
ALTER TABLE `waypoint_path` RENAME COLUMN `waittime` TO `WaitTime`;
ALTER TABLE `waypoint_path` RENAME COLUMN `script_id` TO `ScriptId`;
ALTER TABLE `waypoint_path` RENAME COLUMN `comment` TO `Comment`;
ALTER TABLE `waypoint_path` DROP PRIMARY KEY;
ALTER TABLE `waypoint_path` ADD PRIMARY KEY (`PathId`, `Point`);
ALTER TABLE `waypoint_path` DROP COLUMN `entry`;

ALTER TABLE `creature_movement` RENAME COLUMN `id` TO `Id`;
ALTER TABLE `creature_movement` RENAME COLUMN `point` TO `Point`;
ALTER TABLE `creature_movement` RENAME COLUMN `position_x` TO `PositionX`;
ALTER TABLE `creature_movement` RENAME COLUMN `position_y` TO `PositionY`;
ALTER TABLE `creature_movement` RENAME COLUMN `position_z` TO `PositionZ`;
ALTER TABLE `creature_movement` RENAME COLUMN `orientation` TO `Orientation`;
ALTER TABLE `creature_movement` RENAME COLUMN `waittime` TO `WaitTime`;
ALTER TABLE `creature_movement` RENAME COLUMN `script_id` TO `ScriptId`;
ALTER TABLE `creature_movement` RENAME COLUMN `comment` TO `Comment`;

ALTER TABLE `creature_movement_template` RENAME COLUMN `entry` TO `Entry`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `pathId` TO `PathId`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `point` TO `Point`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `position_x` TO `PositionX`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `position_y` TO `PositionY`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `position_z` TO `PositionZ`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `orientation` TO `Orientation`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `waittime` TO `WaitTime`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `script_id` TO `ScriptId`;
ALTER TABLE `creature_movement_template` RENAME COLUMN `comment` TO `Comment`;

ALTER TABLE `script_waypoint` RENAME COLUMN `entry` TO `Entry`;
ALTER TABLE `script_waypoint` RENAME COLUMN `pathId` TO `PathId`;
ALTER TABLE `script_waypoint` RENAME COLUMN `pointid` TO `Point`;
ALTER TABLE `script_waypoint` RENAME COLUMN `position_x` TO `PositionX`;
ALTER TABLE `script_waypoint` RENAME COLUMN `position_y` TO `PositionY`;
ALTER TABLE `script_waypoint` RENAME COLUMN `position_z` TO `PositionZ`;
ALTER TABLE `script_waypoint` RENAME COLUMN `orientation` TO `Orientation`;
ALTER TABLE `script_waypoint` RENAME COLUMN `waittime` TO `WaitTime`;
ALTER TABLE `script_waypoint` RENAME COLUMN `script_id` TO `ScriptId`;
ALTER TABLE `script_waypoint` RENAME COLUMN `comment` TO `Comment`;

