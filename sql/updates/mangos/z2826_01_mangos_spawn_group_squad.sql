ALTER TABLE db_version CHANGE COLUMN required_z2825_01_mangos_reputation_spillover required_z2826_01_mangos_spawn_group_squad bit;

DROP TABLE IF EXISTS spawn_group_squad;
CREATE TABLE spawn_group_squad(
Id INT NOT NULL COMMENT 'Spawn Group ID',
SquadId INT NOT NULL COMMENT 'Squad Id within Spawn Group',
Guid INT NOT NULL COMMENT 'Guid of creature or GO',
Entry INT NOT NULL COMMENT 'Entry of creature or GO',
PRIMARY KEY(Id, SquadId, Guid)
);

ALTER TABLE spawn_group ADD COLUMN `RespawnOverrideMin` INT UNSIGNED COMMENT 'Respawn time override' DEFAULT NULL AFTER `StringId`;
ALTER TABLE spawn_group ADD COLUMN `RespawnOverrideMax` INT UNSIGNED COMMENT 'Respawn time override' DEFAULT NULL AFTER `RespawnOverrideMin`;


