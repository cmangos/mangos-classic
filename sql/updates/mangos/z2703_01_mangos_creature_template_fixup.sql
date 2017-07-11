ALTER TABLE db_version CHANGE COLUMN required_z2702_01_mangos_spell_chain_totems_typos required_z2703_01_mangos_creature_template_fixup bit;

ALTER TABLE `creature_template` CHANGE COLUMN MinMeleeDmg MinDmg float NOT NULL DEFAULT '0';
ALTER TABLE `creature_template` DROP COLUMN MinRangedDmg;

ALTER TABLE `creature_template` CHANGE COLUMN MaxMeleeDmg MaxDmg float NOT NULL DEFAULT '0';
ALTER TABLE `creature_template` DROP COLUMN MaxRangedDmg;

ALTER TABLE `creature_template` CHANGE COLUMN MeleeAttackPower AttackPower unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_template` DROP COLUMN RangedAttackPower;

ALTER TABLE `creature_template` CHANGE COLUMN MeleeBaseAttackTime BaseAttackTime int(10) unsigned NOT NULL DEFAULT '2000';
ALTER TABLE `creature_template` DROP COLUMN RangedBaseAttackTime;

ALTER TABLE `creature_template` CHANGE COLUMN FactionAlliance Faction smallint(5) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_template` DROP COLUMN FactionHorde;

-- For some reason creature_template_classlevelstats is NOT in the base file?
ALTER TABLE `creature_template_classlevelstats` CHANGE COLUMN BaseMeleeAttackPower BaseAttackPower float NOT NULL DEFAULT '0';
ALTER TABLE `creature_template_classlevelstats` DROP COLUMN BaseRangedAttackPower;
