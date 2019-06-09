ALTER TABLE character_db_version CHANGE COLUMN required_z2735_01_characters_weapon_skills_fix_vanilla required_z2736_01_characters_fix_skills_overflow bit;

UPDATE `character_skills` SET `value` = `max` WHERE `value` > `max`;
