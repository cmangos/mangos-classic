ALTER TABLE realmd_db_version CHANGE COLUMN required_z2678_01_realmd required_z2716_01_realmd_totp bit;

ALTER TABLE account ADD COLUMN token text AFTER locale;
