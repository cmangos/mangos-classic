ALTER TABLE db_version CHANGE COLUMN required_z2824_01_mangos_model_unification required_z2825_01_mangos_reputation_spillover bit;

ALTER TABLE quest_template ADD COLUMN ReputationSpillover tinyint unsigned NOT NULL DEFAULT '1';
