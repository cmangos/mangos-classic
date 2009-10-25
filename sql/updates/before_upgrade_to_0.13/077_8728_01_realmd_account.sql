ALTER TABLE realmd_db_version CHANGE COLUMN required_077_8728_01_realmd_account required_058_7546_02_realmd_uptime bit;

ALTER TABLE account
  DROP COLUMN active_realm_id,
  ADD COLUMN  online  tinyint(4) NOT NULL default '0' AFTER last_login;
