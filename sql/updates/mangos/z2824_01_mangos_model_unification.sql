ALTER TABLE db_version CHANGE COLUMN required_z2823_01_mangos_displayid_probability required_z2824_01_mangos_model_unification bit;

ALTER TABLE `creature_model_info` CHANGE `modelid_other_team` `modelid_alternative` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0;


