ALTER TABLE db_version CHANGE COLUMN required_z2694_01_mangos_instance_encounters required_z2695_01_mangos_instance_template bit;

ALTER TABLE instance_template
ADD COLUMN mountAllowed tinyint(3) UNSIGNED NOT NULL DEFAULT 0 AFTER ScriptName;

UPDATE instance_template SET mountAllowed = 1 WHERE map IN (36, 209, 309, 509);
