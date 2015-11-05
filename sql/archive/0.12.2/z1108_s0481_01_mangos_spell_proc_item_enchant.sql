ALTER TABLE db_version CHANGE COLUMN required_z1101_s0498_02_mangos_creature_template required_z1108_s0481_01_mangos_spell_proc_item_enchant bit;

DROP TABLE IF EXISTS `spell_proc_item_enchant`;
CREATE TABLE `spell_proc_item_enchant` (
  `entry` mediumint unsigned NOT NULL,
  `ppmRate` float NOT NULL default '0',
  PRIMARY KEY  (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO `spell_proc_item_enchant` (`entry`, `ppmRate`) VALUES
(8034, 9);        -- Frostbrand Weapon
