ALTER TABLE db_version CHANGE COLUMN required_z2834_01_mangos_spillover_rename required_z2835_01_mangos_pet_spell_lists bit;

DROP TABLE IF EXISTS pet_autocast_spell_list;
CREATE TABLE pet_autocast_spell_list (
`CreatureEntry` INT UNSIGNED NOT NULL,
`SpellId` INT UNSIGNED NOT NULL,
`CombatCondition` INT NOT NULL DEFAULT '-1',
`TargetId` INT NOT NULL,
`Comments` VARCHAR(255) NOT NULL,
PRIMARY KEY(`CreatureEntry`, `SpellId`)
);

