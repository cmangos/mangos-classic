ALTER TABLE db_version CHANGE COLUMN required_s2431_01_mangos_corpse_decay required_s2432_01_mangos_gameobject_spawn_entry bit;

DROP TABLE IF EXISTS gameobject_spawn_entry;
CREATE TABLE gameobject_spawn_entry(
`guid` INT UNSIGNED NOT NULL COMMENT 'Gameobject table guid',
`entry` MEDIUMINT UNSIGNED NOT NULL COMMENT 'Gameobject Template entry',
PRIMARY KEY(`guid`,`entry`)
);

