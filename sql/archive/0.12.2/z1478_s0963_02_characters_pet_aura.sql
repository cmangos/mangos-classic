ALTER TABLE character_db_version CHANGE COLUMN required_z1478_s0963_01_characters_character_aura required_z1478_s0963_02_characters_pet_aura bit;

DROP TABLE IF EXISTS `pet_aura`;
CREATE TABLE `pet_aura` (
  `guid` int(11) unsigned NOT NULL default '0' COMMENT 'Global Unique Identifier',
  `caster_guid` bigint(20) unsigned NOT NULL default '0' COMMENT 'Full Global Unique Identifier',
  `item_guid` int(11) unsigned NOT NULL default '0',
  `spell` int(11) unsigned NOT NULL default '0',
  `stackcount` int(11) NOT NULL default '1',
  `remaincharges` int(11) NOT NULL default '0',
  `basepoints0` INT(11) NOT NULL DEFAULT '0',
  `basepoints1` INT(11) NOT NULL DEFAULT '0',
  `basepoints2` INT(11) NOT NULL DEFAULT '0',
  `maxduration0` INT(11) NOT NULL DEFAULT '0',
  `maxduration1` INT(11) NOT NULL DEFAULT '0',
  `maxduration2` INT(11) NOT NULL DEFAULT '0',
  `remaintime0` INT(11) NOT NULL DEFAULT '0',
  `remaintime1` INT(11) NOT NULL DEFAULT '0',
  `remaintime2` INT(11) NOT NULL DEFAULT '0',
  `effIndexMask` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`caster_guid`,`item_guid`,`spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Pet System';
