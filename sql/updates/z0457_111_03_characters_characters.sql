ALTER TABLE character_db_version CHANGE COLUMN required_z0437_106_01_characters required_z0457_111_03_characters_characters bit;

ALTER TABLE `characters`
   ADD COLUMN `deleteInfos_Account` int(11) UNSIGNED default NULL AFTER stored_honorable_kills,
   ADD COLUMN `deleteInfos_Name` varchar(12) default NULL AFTER deleteInfos_Account,
   ADD COLUMN `deleteDate` bigint(20) default NULL AFTER deleteInfos_Name;
