ALTER TABLE realmd_db_version CHANGE COLUMN required_z2778_01_realmd_anticheat required_z2800_01_realmd_platform bit;

ALTER TABLE account ADD COLUMN `platform` VARCHAR(4) NOT NULL DEFAULT '0' AFTER `os`;


