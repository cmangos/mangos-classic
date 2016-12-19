ALTER TABLE db_version CHANGE COLUMN required_z2692_02_mangos_quest_template required_z2692_03_mangos_npc_trainer bit;

ALTER TABLE npc_trainer ADD COLUMN `condition_id` INT(11) unsigned NOT NULL default '0' AFTER reqlevel;
