ALTER TABLE db_version CHANGE COLUMN required_z2814_01_mangos_worldstate_expression_spawn_group required_z2815_01_mangos_pursuit bit;

UPDATE creature_template SET Pursuit=15000 WHERE Pursuit=0;


