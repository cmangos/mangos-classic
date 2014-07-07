ALTER TABLE db_version CHANGE COLUMN required_z2683_01_mangos_scriptdev2_tables required_z2684_01_mangos_spell_elixir bit;

DROP TABLE IF EXISTS `spell_elixir`;
CREATE TABLE `spell_elixir` (
  `entry` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'SpellId of potion',
  `mask` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT 'Mask 0x1 flask 0x2 zanza elixir',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Spell System';

INSERT INTO `spell_elixir` VALUES
(17624,1), -- Flask of Petrification
(17626,1), -- Flask of the Titans
(17627,1), -- Flask of Distilled Wisdom
(17628,1), -- Flask of Supreme Power
(17629,1), -- Flask of Chromatic Resistance
(24382,2), -- Spirit of Zanza
(24383,2), -- Swiftness of Zanza
(24417,2); -- Sheen of Zanza
