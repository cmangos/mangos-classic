ALTER TABLE character_db_version CHANGE COLUMN required_z0889_s0224_02_characters_gameobject_respawn required_z1014_s0386_01_characters_item_loot bit;

DROP TABLE IF EXISTS `item_loot`;
CREATE TABLE `item_loot` (
  `guid` int(11) unsigned NOT NULL default '0',
  `owner_guid` int(11) unsigned NOT NULL default '0',
  `itemid` int(11) unsigned NOT NULL default '0',
  `amount` int(11) unsigned NOT NULL default '0',
  `property` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guid`,`itemid`),
  KEY `idx_owner_guid` (`owner_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Item System';
