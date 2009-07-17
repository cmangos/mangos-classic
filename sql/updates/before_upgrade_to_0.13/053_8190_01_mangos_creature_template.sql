ALTER TABLE db_version CHANGE COLUMN required_053_8190_01_mangos_creature_template required_048_7382_01_mangos_creature_template bit;

ALTER TABLE `creature_template`
    DROP COLUMN `KillCredit1`,
    DROP COLUMN `KillCredit2`;
