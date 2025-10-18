DROP TABLE IF EXISTS `ai_playerbot_equip_cache`;
CREATE TABLE `ai_playerbot_equip_cache` (
  `id` bigint auto_increment,
  `clazz` mediumint(8) NOT NULL,
  `spec` mediumint(8) NOT NULL,
  `lvl` mediumint(8) NOT NULL,
  `slot` mediumint(8) NOT NULL,
  `quality` mediumint(8) NOT NULL,
  `item` mediumint(8) NOT NULL,
PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI equip Cache';

DROP TABLE IF EXISTS `ai_playerbot_rarity_cache`;
CREATE TABLE `ai_playerbot_rarity_cache` (
  `id` bigint auto_increment,
  `item` mediumint(8) NOT NULL,
  `rarity` float NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI Rarity Cache';

DROP TABLE IF EXISTS `ai_playerbot_rnditem_cache`;
CREATE TABLE `ai_playerbot_rnditem_cache` (
  `id` bigint auto_increment,
  `lvl` mediumint(8) NOT NULL,
  `type` mediumint(8) NOT NULL,
  `item` mediumint(8) NOT NULL,
PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI Random Item Cache';

DROP TABLE IF EXISTS `ai_playerbot_tele_cache`;
CREATE TABLE `ai_playerbot_tele_cache` (
  `id` mediumint(8) auto_increment,
  `level` mediumint(8) NOT NULL,
  `map_id` mediumint(8) NOT NULL,
  `x` float(8) NOT NULL,
  `y` float(8) NOT NULL,
  `z` float(8) NOT NULL,
PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI Tele Cache';

DROP TABLE IF EXISTS `ai_playerbot_item_info_cache`;
CREATE TABLE IF NOT EXISTS `ai_playerbot_item_info_cache` (
  `id` bigint(20) NOT NULL,
  `quality` bigint(20) DEFAULT NULL,
  `slot` bigint(20) DEFAULT NULL,
  `source` mediumint(8) DEFAULT NULL,
  `sourceId` mediumint(8) DEFAULT NULL,
  `team` mediumint(8) DEFAULT NULL,
  `faction` mediumint(8) DEFAULT NULL,
  `factionRepRank` mediumint(8) DEFAULT NULL,
  `minLevel` mediumint(8) DEFAULT NULL,
  `scale_1` mediumint(8) DEFAULT NULL,
  `scale_2` mediumint(8) DEFAULT NULL,
  `scale_3` mediumint(8) DEFAULT NULL,
  `scale_4` mediumint(8) DEFAULT NULL,
  `scale_5` mediumint(8) DEFAULT NULL,
  `scale_6` mediumint(8) DEFAULT NULL,
  `scale_7` mediumint(8) DEFAULT NULL,
  `scale_8` mediumint(8) DEFAULT NULL,
  `scale_9` mediumint(8) DEFAULT NULL,
  `scale_10` mediumint(8) DEFAULT NULL,
  `scale_11` mediumint(8) DEFAULT NULL,
  `scale_12` mediumint(8) DEFAULT NULL,
  `scale_13` mediumint(8) DEFAULT NULL,
  `scale_14` mediumint(8) DEFAULT NULL,
  `scale_15` mediumint(8) DEFAULT NULL,
  `scale_16` mediumint(8) DEFAULT NULL,
  `scale_17` mediumint(8) DEFAULT NULL,
  `scale_18` mediumint(8) DEFAULT NULL,
  `scale_19` mediumint(8) DEFAULT NULL,
  `scale_20` mediumint(8) DEFAULT NULL,
  `scale_21` mediumint(8) DEFAULT NULL,
  `scale_22` mediumint(8) DEFAULT NULL,
  `scale_23` mediumint(8) DEFAULT NULL,
  `scale_24` mediumint(8) DEFAULT NULL,
  `scale_25` mediumint(8) DEFAULT NULL,
  `scale_26` mediumint(8) DEFAULT NULL,
  `scale_27` mediumint(8) DEFAULT NULL,
  `scale_28` mediumint(8) DEFAULT NULL,
  `scale_29` mediumint(8) DEFAULT NULL,
  `scale_30` mediumint(8) DEFAULT NULL,
  `scale_31` mediumint(8) DEFAULT NULL,
  `scale_32` mediumint(8) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI item info Cache';

