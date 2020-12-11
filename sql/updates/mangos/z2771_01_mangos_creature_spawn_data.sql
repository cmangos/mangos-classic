ALTER TABLE db_version CHANGE COLUMN required_z2769_01_mangos_creature_template_spells_extension required_z2771_01_mangos_creature_spawn_data bit;

DROP TABLE IF EXISTS `creature_spawn_data_template`;
CREATE TABLE creature_spawn_data_template(
Entry INT UNSIGNED NOT NULL COMMENT 'ID of template',
UnitFlags BIGINT NOT NULL DEFAULT '-1',
Faction INT UNSIGNED NOT NULL DEFAULT '0',
ModelId MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
EquipmentId MEDIUMINT NOT NULL DEFAULT '0',
CurHealth INT UNSIGNED NOT NULL DEFAULT '1',
CurMana INT UNSIGNED NOT NULL DEFAULT '0',
SpawnFlags INT UNSIGNED NOT NULL DEFAULT '0',
PRIMARY KEY(Entry,UnitFlags,ModelId,EquipmentId,CurHealth,CurMana,SpawnFlags)
);

DROP TABLE IF EXISTS `creature_spawn_data`;
CREATE TABLE creature_spawn_data(
Guid INT UNSIGNED NOT NULL COMMENT 'guid of creature',
Id INT UNSIGNED NOT NULL COMMENT 'ID of template',
PRIMARY KEY(Guid)
);

INSERT INTO creature_spawn_data_template(entry) VALUES(0);
