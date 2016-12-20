ALTER TABLE db_version CHANGE COLUMN required_z2692_01_mangos_areatrigger_teleport required_z2692_02_mangos_quest_template bit;

ALTER TABLE quest_template ADD COLUMN RequiredCondition INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER RequiredSkillValue;
