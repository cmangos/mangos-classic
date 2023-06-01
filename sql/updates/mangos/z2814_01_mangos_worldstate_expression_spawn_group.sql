ALTER TABLE db_version CHANGE COLUMN required_z2813_01_mangos_trainer_ability required_z2814_01_mangos_worldstate_expression_spawn_group bit;

ALTER TABLE `spawn_group` ADD COLUMN `WorldStateExpression` INT(11) NOT NULL DEFAULT 0 AFTER `WorldState`;


