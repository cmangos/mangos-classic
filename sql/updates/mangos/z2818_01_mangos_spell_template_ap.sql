ALTER TABLE db_version CHANGE COLUMN required_z2817_01_mangos_spell_template required_z2818_01_mangos_spell_template_ap bit;

ALTER TABLE `spell_template`
	ADD COLUMN `EffectBonusCoefficientFromAP1` FLOAT NOT NULL DEFAULT '0' AFTER `EffectBonusCoefficient3`,
	ADD COLUMN `EffectBonusCoefficientFromAP2` FLOAT NOT NULL DEFAULT '0' AFTER `EffectBonusCoefficientFromAP1`,
	ADD COLUMN `EffectBonusCoefficientFromAP3` FLOAT NOT NULL DEFAULT '0' AFTER `EffectBonusCoefficientFromAP2`;


