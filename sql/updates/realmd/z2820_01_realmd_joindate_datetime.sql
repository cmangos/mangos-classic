ALTER TABLE realmd_db_version CHANGE COLUMN required_z2800_01_realmd_platform required_z2820_01_realmd_joindate_datetime bit;

ALTER TABLE `account` MODIFY joindate DATETIME NOT NULL DEFAULT NOW();


