ALTER TABLE character_db_version CHANGE COLUMN required_z2753_01_characters_new_ticket_system_sql_created required_z2764_01_characters_ahbot_items bit;

DROP TABLE IF EXISTS `ahbot_items`;
CREATE TABLE `ahbot_items` (
  `item` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Item Identifier',
  `value` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Item value, a value of 0 bans item from being sold/bought by AHBot',
  `add_chance` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Chance item is added to AH upon bot visit, 0 for normal loot sources',
  `min_amount` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Min amount added, not used when add_chance is 0',
  `max_amount` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Max amount added, not used when add_chance is 0',
  PRIMARY KEY (`item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='AuctionHouseBot overridden item settings';
