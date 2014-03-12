DROP TABLE IF EXISTS `creature_template_classlevelstats`;

CREATE TABLE `creature_template_classlevelstats` (
  `Level` tinyint(4) NOT NULL,
  `Class` tinyint(4) NOT NULL,
  `BaseHealthExp0` mediumint(8) unsigned NOT NULL DEFAULT '1',
  `BaseMana` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `BaseDamageExp0` float NOT NULL DEFAULT '0',
  `BaseMeleeAttackPower` float NOT NULL DEFAULT '0',
  `BaseRangedAttackPower` float NOT NULL DEFAULT '0',
  `BaseArmor` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `STR` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `AGI` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `STA` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `INT` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SPI` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Level`,`Class`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
