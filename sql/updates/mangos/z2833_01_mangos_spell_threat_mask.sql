ALTER TABLE db_version CHANGE COLUMN required_z2832_01_mangos_dbscript_breaking_change required_z2833_01_mangos_spell_threat_mask bit;

ALTER TABLE `spell_threat` ADD COLUMN `inverseEffectMask` INT UNSIGNED DEFAULT '0' AFTER `ap_bonus`;

