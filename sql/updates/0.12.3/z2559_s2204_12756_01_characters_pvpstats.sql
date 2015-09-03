ALTER TABLE character_db_version CHANGE COLUMN required_z2410_s2063_12562_01_characters_various_tables required_z2559_s2204_12756_01_characters_pvpstats bit;

DROP TABLE IF EXISTS `pvpstats_battlegrounds`;
CREATE TABLE `pvpstats_battlegrounds` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `winner_team` TINYINT NOT NULL,
  `bracket_id` TINYINT UNSIGNED NOT NULL,
  `type` TINYINT UNSIGNED NOT NULL,
  `date` DATETIME NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS `pvpstats_players`;
CREATE TABLE `pvpstats_players` (
  `battleground_id` BIGINT UNSIGNED NOT NULL,
  `player_guid` INT UNSIGNED NOT NULL,
  `score_killing_blows` MEDIUMINT UNSIGNED NOT NULL,
  `score_deaths` MEDIUMINT UNSIGNED NOT NULL,
  `score_honorable_kills` MEDIUMINT UNSIGNED NOT NULL,
  `score_bonus_honor` MEDIUMINT UNSIGNED NOT NULL,
  `score_damage_done` MEDIUMINT UNSIGNED NOT NULL,
  `score_healing_done` MEDIUMINT UNSIGNED NOT NULL,
  `attr_1` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  `attr_2` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  `attr_3` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  `attr_4` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  `attr_5` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`battleground_id`, `player_guid`)
) ENGINE=InnoDB;
