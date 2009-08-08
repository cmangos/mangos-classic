ALTER TABLE realmd_db_version CHANGE COLUMN required_055_8332_01_realmd_realmcharacters required_2008_11_07_04_realmd_account bit;

ALTER TABLE realmcharacters
  DROP KEY acctid;
