ALTER TABLE db_version CHANGE COLUMN required_z2821_01_mangos_charmed_spell_list required_z2822_01_mangos_creature_cls_stats bit;

ALTER TABLE creature_template_classlevelstats ADD `Strength` int(11) NOT NULL DEFAULT '0';
ALTER TABLE creature_template_classlevelstats ADD `Agility` int(11) NOT NULL DEFAULT '0';
ALTER TABLE creature_template_classlevelstats ADD `Stamina` int(11) NOT NULL DEFAULT '0';
ALTER TABLE creature_template_classlevelstats ADD `Intellect` int(11) NOT NULL DEFAULT '0';
ALTER TABLE creature_template_classlevelstats ADD `Spirit` int(11) NOT NULL DEFAULT '0';

ALTER TABLE creature_template ADD `StrengthMultiplier` FLOAT NOT NULL DEFAULT 1 AFTER `ExperienceMultiplier`;
ALTER TABLE creature_template ADD `AgilityMultiplier` FLOAT NOT NULL DEFAULT 1 AFTER `StrengthMultiplier`;
ALTER TABLE creature_template ADD `StaminaMultiplier` FLOAT NOT NULL DEFAULT 1 AFTER `AgilityMultiplier`;
ALTER TABLE creature_template ADD `IntellectMultiplier` FLOAT NOT NULL DEFAULT 1 AFTER `StaminaMultiplier`;
ALTER TABLE creature_template ADD `SpiritMultiplier` FLOAT NOT NULL DEFAULT 1 AFTER `IntellectMultiplier`;


