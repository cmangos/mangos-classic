ALTER TABLE db_version CHANGE COLUMN required_z2830_01_mangos_icon_name required_z2831_01_mangos_wmogroupgen bit;

ALTER TABLE creature_zone ADD COLUMN WmoGroupId INT DEFAULT 0 AFTER `AreaId`;
ALTER TABLE gameobject_zone ADD COLUMN WmoGroupId INT DEFAULT 0 AFTER `AreaId`;

