ALTER TABLE realmd_db_version CHANGE COLUMN required_z2716_01_realmd_totp required_z2739_01_realmd_pin bit;

ALTER TABLE account DROP COLUMN token;
ALTER TABLE account ADD COLUMN `token` varchar(16) DEFAULT NULL AFTER locale;
ALTER TABLE account ADD COLUMN `security` int(11) DEFAULT '0' AFTER locale;
