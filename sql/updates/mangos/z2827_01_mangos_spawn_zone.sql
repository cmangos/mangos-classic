ALTER TABLE db_version CHANGE COLUMN required_z2826_01_mangos_spawn_group_squad required_z2827_01_mangos_spawn_zone bit;

DROP TABLE IF EXISTS `creature_zone`;
CREATE TABLE `creature_zone` (
  `Guid` int unsigned NOT NULL AUTO_INCREMENT COMMENT 'Global Unique Identifier',
  `ZoneId` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Zone Identifier',
  `AreaId` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Area Identifier',
  PRIMARY KEY(`Guid`)
);

DROP TABLE IF EXISTS `gameobject_zone`;
CREATE TABLE `gameobject_zone` (
  `Guid` int unsigned NOT NULL AUTO_INCREMENT COMMENT 'Global Unique Identifier',
  `ZoneId` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Zone Identifier',
  `AreaId` mediumint unsigned NOT NULL DEFAULT '0' COMMENT 'Area Identifier',
  PRIMARY KEY(`Guid`)
);

