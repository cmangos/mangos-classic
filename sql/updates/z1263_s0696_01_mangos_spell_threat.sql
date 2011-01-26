ALTER TABLE db_version CHANGE COLUMN required_z1247_s0693_01_mangos_command required_z1263_s0696_01_mangos_spell_threat bit;

ALTER TABLE spell_threat ADD COLUMN multiplier FLOAT NOT NULL DEFAULT 1.0 COMMENT 'threat multiplier for damage/healing' AFTER Threat;

ALTER TABLE spell_threat ADD COLUMN ap_bonus FLOAT NOT NULL DEFAULT 0.0 COMMENT 'additional threat bonus from attack power' AFTER multiplier;

DELETE FROM spell_threat WHERE entry IN (
 1672, 20569, 11556, 14921, 11775, 25289, 24583,
 778, 9749, 9907, 17390, 17391, 17392,
 72, 845, 1160, 2649, 3716, 6673, 6807, 24640
);

INSERT INTO `spell_threat` VALUES
(   72, 180, 1, 0),
(  845, 100, 1, 0),
( 1160,  43, 1, 0),
( 2649, 415, 1, 0),
( 3716, 395, 1, 0),
( 6673,  60, 1, 0),
( 6807,  59, 1, 0),
(24640,   5, 1, 0);
