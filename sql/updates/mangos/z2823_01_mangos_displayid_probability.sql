ALTER TABLE db_version CHANGE COLUMN required_z2822_01_mangos_creature_cls_stats required_z2823_01_mangos_displayid_probability bit;

ALTER TABLE `creature_template` CHANGE `ModelId1` `DisplayId1` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0;
ALTER TABLE `creature_template` CHANGE `ModelId2` `DisplayId2` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0;
ALTER TABLE `creature_template` CHANGE `ModelId3` `DisplayId3` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0;
ALTER TABLE `creature_template` CHANGE `ModelId4` `DisplayId4` MEDIUMINT UNSIGNED NOT NULL DEFAULT 0;
ALTER TABLE `creature_template` ADD COLUMN `DisplayIdProbability1` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `DisplayId4`;
ALTER TABLE `creature_template` ADD COLUMN `DisplayIdProbability2` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `DisplayIdProbability1`;
ALTER TABLE `creature_template` ADD COLUMN `DisplayIdProbability3` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `DisplayIdProbability2`;
ALTER TABLE `creature_template` ADD COLUMN `DisplayIdProbability4` SMALLINT UNSIGNED NOT NULL DEFAULT 0 AFTER `DisplayIdProbability3`;

SET sql_safe_updates=0;
-- setting probabilities to exactly replicate previous behaviour
UPDATE creature_template SET DisplayIdProbability1=100 WHERE DisplayId1!=0;
UPDATE creature_template SET DisplayIdProbability1=50,DisplayIdProbability2=50 WHERE DisplayId1!=0 AND DisplayId2!=0 AND EXISTS(SELECT modelid FROM creature_model_info WHERE creature_template.DisplayId2=creature_model_info.modelid AND modelid_other_team > 0);
UPDATE creature_template SET DisplayIdProbability1=0,DisplayIdProbability2=100 WHERE DisplayId1!=0 AND DisplayId2!=0 AND NOT EXISTS(SELECT modelid FROM creature_model_info WHERE creature_template.DisplayId2=creature_model_info.modelid AND modelid_other_team > 0);
UPDATE creature_template SET DisplayIdProbability1=33,DisplayIdProbability2=33,DisplayIdProbability3=33 WHERE DisplayId1!=0 AND DisplayId2!=0 AND DisplayId3!=0 AND EXISTS(SELECT modelid FROM creature_model_info WHERE creature_template.DisplayId2=creature_model_info.modelid AND modelid_other_team > 0);
UPDATE creature_template SET DisplayIdProbability1=0,DisplayIdProbability2=0,DisplayIdProbability3=100 WHERE DisplayId1!=0 AND DisplayId2!=0 AND DisplayId3!=0 AND NOT EXISTS(SELECT modelid FROM creature_model_info WHERE creature_template.DisplayId2=creature_model_info.modelid AND modelid_other_team > 0);
UPDATE creature_template SET DisplayIdProbability1=25,DisplayIdProbability2=25,DisplayIdProbability3=25,DisplayIdProbability4=25 WHERE DisplayId1!=0 AND DisplayId2!=0 AND DisplayId3!=0 AND DisplayId4!=0;
SET sql_safe_updates=1;
