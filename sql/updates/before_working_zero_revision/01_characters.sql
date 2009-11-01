ALTER TABLE `characters` 
ADD COLUMN `honor_highest_rank` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `taxi_path`,
ADD COLUMN `honor_standing` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `honor_highest_rank`,
ADD COLUMN `honor_rating` FLOAT UNSIGNED NOT NULL DEFAULT '0' AFTER `honor_standing`;



CREATE TABLE `character_kill` (
  `guid` INT(11) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `creature_template` INT(11) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Creature Identifier',
  `honor` FLOAT NOT NULL DEFAULT '0',
  `date` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `type` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  KEY `idx_guid` (`guid`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Player System';

ALTER TABLE characters MODIFY `honor_rating` FLOAT NOT NULL DEFAULT '0';