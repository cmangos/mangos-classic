ALTER TABLE db_version CHANGE COLUMN required_z2810_01_mangos_spell_list_conditions required_z2811_01_mangos_creature_static_flags bit;

ALTER TABLE creature_template ADD COLUMN `StaticFlags1` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `CreatureTypeFlags`;
ALTER TABLE creature_template ADD COLUMN `StaticFlags2` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `StaticFlags1`;
ALTER TABLE creature_template ADD COLUMN `StaticFlags3` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `StaticFlags2`;
ALTER TABLE creature_template ADD COLUMN `StaticFlags4` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `StaticFlags3`;

-- ports from extraflags
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00000020,StaticFlags2=StaticFlags2|0x00000010 WHERE ExtraFlags&0x00000020;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00000040,StaticFlags1=StaticFlags1|0x00000002 WHERE ExtraFlags&0x00000040;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00000100,StaticFlags2=StaticFlags2|0x00000040 WHERE ExtraFlags&0x00000100;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x00400000,StaticFlags2=StaticFlags2|0x00000100 WHERE ExtraFlags&0x00400000;
UPDATE creature_template SET ExtraFlags=ExtraFlags&~0x01000000,StaticFlags2=StaticFlags2|0x40000000 WHERE ExtraFlags&0x01000000;

