ALTER TABLE db_version CHANGE COLUMN required_z2692_03_mangos_npc_trainer required_z2692_04_mangos_npc_trainer_template bit;

ALTER TABLE npc_trainer_template ADD COLUMN `condition_id` INT(11) unsigned NOT NULL default '0' AFTER reqlevel;
