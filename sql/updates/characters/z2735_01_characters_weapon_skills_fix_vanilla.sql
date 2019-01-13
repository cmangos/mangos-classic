ALTER TABLE character_db_version CHANGE COLUMN required_z2734_01_characters_skills_cleanup_1829 required_z2735_01_characters_weapon_skills_fix_vanilla bit;

DELETE a FROM character_skills AS a WHERE skill IN (118) AND guid IN (SELECT guid FROM characters b WHERE class IN (4) AND level < 10 AND a.guid=b.guid);
