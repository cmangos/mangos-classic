ALTER TABLE realmd_db_version CHANGE COLUMN required_2008_11_07_04_realmd_account required_055_8332_01_realmd_realmcharacters bit;

ALTER TABLE realmcharacters
  ADD KEY (acctid);
