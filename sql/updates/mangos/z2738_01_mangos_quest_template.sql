ALTER TABLE db_version CHANGE COLUMN required_z2735_01_mangos_weapon_skills_fix_vanilla required_z2738_01_mangos_quest_template bit;

ALTER TABLE quest_template ADD COLUMN MaxLevel tinyint(3) unsigned NOT NULL DEFAULT '255' AFTER MinLevel;
