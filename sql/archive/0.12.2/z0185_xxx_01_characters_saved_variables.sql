ALTER TABLE character_db_version CHANGE COLUMN required_z0169_091_01_characters_group required_z0185_xxx_01_characters_saved_variables bit;
DROP TABLE IF EXISTS `saved_variables`;
CREATE TABLE `saved_variables` (
    `NextMaintenanceDate` int(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Variable Saves';
