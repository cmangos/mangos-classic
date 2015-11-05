ALTER TABLE db_version CHANGE COLUMN required_z1840_s1399_11813_01_mangos_mangos_string required_z1852_s1413_11827_01_mangos_creature_linking_template bit;

DROP TABLE IF EXISTS creature_linking_template;
CREATE TABLE creature_linking_template (
  entry INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'creature_template.entry of the slave mob that is linked',
  map MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'Id of map of the mobs',
  master_entry INT(10) UNSIGNED NOT NULL COMMENT 'master to trigger events',
  flag MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'flag - describing what should happen when',
  PRIMARY KEY (entry, map)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Creature Linking System';
