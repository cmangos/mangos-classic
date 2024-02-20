ALTER TABLE db_version CHANGE COLUMN required_z2816_01_mangos_precision_decimal required_z2817_01_mangos_spell_template bit;

ALTER TABLE `spell_template`
	ADD COLUMN `EffectBonusCoefficient1` FLOAT NOT NULL DEFAULT '0' AFTER `RequiredAuraVision`,
	ADD COLUMN `EffectBonusCoefficient2` FLOAT NOT NULL DEFAULT '0' AFTER `EffectBonusCoefficient1`,
	ADD COLUMN `EffectBonusCoefficient3` FLOAT NOT NULL DEFAULT '0' AFTER `EffectBonusCoefficient2`;

DROP TABLE `spell_bonus_data`;
