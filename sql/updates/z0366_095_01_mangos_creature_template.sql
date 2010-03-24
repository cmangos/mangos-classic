ALTER TABLE db_version CHANGE COLUMN required_z0332_xxx_01_mangos_gossip_menu_option required_z0366_095_01_mangos_creature_template bit;

ALTER TABLE creature_template ADD COLUMN speed_run float NOT NULL default '1.14286' COMMENT 'Result of 8.0/7.0, most common value' AFTER speed;
ALTER TABLE creature_template CHANGE COLUMN speed speed_walk float NOT NULL default '1' COMMENT 'Result of 2.5/2.5, most common value';
