ALTER TABLE db_version CHANGE COLUMN required_z2745_01_mangos_game_event_split required_z2746_01_mangos_world_safe_locs_facing bit;

CREATE TABLE IF NOT EXISTS `world_safe_locs` (
  `id` int(11) unsigned NOT NULL,
  `map` int(10) unsigned NOT NULL DEFAULT '0',
  `x` float NOT NULL DEFAULT '0',
  `y` float NOT NULL DEFAULT '0',
  `z` float NOT NULL DEFAULT '0',
  `o` float NOT NULL DEFAULT '0',
  `name` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `battleground_template`
	DROP COLUMN `AllianceStartO`,
	DROP COLUMN `HordeStartO`;
