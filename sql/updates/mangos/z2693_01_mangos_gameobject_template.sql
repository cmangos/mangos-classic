ALTER TABLE db_version CHANGE COLUMN required_z2692_04_mangos_npc_trainer_template required_z2693_01_mangos_gameobject_template bit;

ALTER TABLE gameobject_template
ADD COLUMN ExtraFlags INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER flags,
ADD COLUMN CustomData1 INT(11) UNSIGNED NOT NULL DEFAULT 0 AFTER data23;
