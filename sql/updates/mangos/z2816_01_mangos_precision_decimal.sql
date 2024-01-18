ALTER TABLE db_version CHANGE COLUMN required_z2815_01_mangos_pursuit required_z2816_01_mangos_precision_decimal bit;

ALTER TABLE gameobject CHANGE position_x position_x DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE position_y position_y DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE position_z position_z DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE orientation orientation DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE rotation0 rotation0 DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE rotation1 rotation1 DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE rotation2 rotation2 DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE gameobject CHANGE rotation3 rotation3 DECIMAL(40,20) NOT NULL DEFAULT 0;

ALTER TABLE creature CHANGE position_x position_x DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE creature CHANGE position_y position_y DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE creature CHANGE position_z position_z DECIMAL(40,20) NOT NULL DEFAULT 0;
ALTER TABLE creature CHANGE orientation orientation DECIMAL(40,20) NOT NULL DEFAULT 0;
