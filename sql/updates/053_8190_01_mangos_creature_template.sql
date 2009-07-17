ALTER TABLE db_version CHANGE COLUMN required_048_7382_01_mangos_creature_template required_053_8190_01_mangos_creature_template bit;

ALTER TABLE `creature_template`
    ADD COLUMN `KillCredit1` int(11) unsigned NOT NULL default '0' AFTER `heroic_entry`,
    ADD COLUMN `KillCredit2` int(11) unsigned NOT NULL default '0' AFTER `KillCredit1`;
