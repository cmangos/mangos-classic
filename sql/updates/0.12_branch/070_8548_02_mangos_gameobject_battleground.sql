
DROP TABLE IF EXISTS `gameobject_battleground`;
CREATE TABLE `gameobject_battleground` (
    `guid` int(10) unsigned NOT NULL COMMENT 'GameObject\'s GUID',
    `event1` tinyint(3) unsigned NOT NULL COMMENT 'main event',
    `event2` tinyint(3) unsigned NOT NULL COMMENT 'sub event',
    PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='GameObject battleground indexing system';
