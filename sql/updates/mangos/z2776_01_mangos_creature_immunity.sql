ALTER TABLE db_version CHANGE COLUMN required_z2774_01_mangos_column_fix required_z2776_01_mangos_creature_immunity bit;

DROP TABLE IF EXISTS creature_immunities;
CREATE TABLE creature_immunities(
`Entry` INT UNSIGNED NOT NULL COMMENT 'creature_template entry',
`SetId` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'immunity set ID',
`Type` TINYINT UNSIGNED NOT NULL COMMENT 'enum SpellImmunity',
`Value` INT UNSIGNED NOT NULL COMMENT 'value depending on type',
PRIMARY KEY(`Entry`,`SetId`,`Type`,`Value`)
);


-- taunt effect and aura
INSERT INTO creature_immunities SELECT entry, 0, 0, 114 FROM creature_template WHERE ExtraFlags&0x00000100;
INSERT INTO creature_immunities SELECT entry, 0, 1, 11 FROM creature_template WHERE ExtraFlags&0x00000100;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00000100 WHERE ExtraFlags&0x00000100;
-- haste auras
INSERT INTO creature_immunities SELECT entry, 0, 1, 216 FROM creature_template WHERE ExtraFlags&0x00400000;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00400000 WHERE ExtraFlags&0x00400000;
-- poison auras
INSERT INTO creature_immunities SELECT entry, 0, 4, 4 FROM creature_template WHERE ExtraFlags&0x01000000;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x01000000 WHERE ExtraFlags&0x01000000;
