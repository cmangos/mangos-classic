ALTER TABLE character_db_version CHANGE COLUMN required_z0435_105_01_characters_character_stats required_z0437_106_01_characters bit;

ALTER TABLE `character_spell` ADD KEY `Idx_spell` (`spell`);
