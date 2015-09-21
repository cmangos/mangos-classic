ALTER TABLE db_version CHANGE COLUMN required_z0742_s0046_03_mangos_pool_pool required_z0759_s0065_01_mangos_creature_addon bit;

ALTER TABLE creature_addon
  CHANGE `guid` `guid` int(10) unsigned NOT NULL default '0';
