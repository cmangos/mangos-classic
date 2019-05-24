ALTER TABLE db_version CHANGE COLUMN required_z2721_01_mangos_creature_template_spells_extension required_z2722_01_mangos_creature_waypoint_refurbishing bit;

ALTER TABLE creature_movement ADD COLUMN comment text;
ALTER TABLE creature_movement_template ADD COLUMN comment text;
ALTER TABLE creature_movement MODIFY COLUMN orientation float NOT NULL DEFAULT '0' AFTER position_z;
ALTER TABLE creature_movement_template MODIFY COLUMN orientation float NOT NULL DEFAULT '0' AFTER position_z;
ALTER TABLE script_waypoint CHANGE COLUMN point_comment comment text;
ALTER TABLE script_waypoint CHANGE COLUMN location_x position_x text;
ALTER TABLE script_waypoint CHANGE COLUMN location_y position_y text;
ALTER TABLE script_waypoint CHANGE COLUMN location_z position_z text;
ALTER TABLE script_waypoint ADD COLUMN orientation float NOT NULL DEFAULT '0' AFTER position_z;
ALTER TABLE script_waypoint ADD COLUMN pathId INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER entry;
ALTER TABLE script_waypoint ADD COLUMN script_id MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER waittime;
ALTER TABLE script_waypoint DROP PRIMARY KEY;
ALTER TABLE script_waypoint ADD PRIMARY KEY (entry, pathId, pointId);

/*
select concat('(',entry,',',pathId,',',pointId,',',position_x,',',position_y,',',position_z,',',orientation,',',waittime,',',script_id,',''',comment,'''),') from script_waypoint ORDER BY entry, pathId, pointId LIMIT 0, 5000;
*/