ALTER TABLE realmd_db_version CHANGE COLUMN required_z2678_01_realmd required_z2685_01_realmd BIT(1) NULL DEFAULT NULL;

ALTER TABLE account ADD COLUMN security VARCHAR(16) NULL DEFAULT NULL AFTER locale;