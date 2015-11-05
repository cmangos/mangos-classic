ALTER TABLE realmd_db_version CHANGE COLUMN required_z0001_xxx_03_realmd_realmd_db_version required_9010_01_realmd_realmlist bit;

ALTER TABLE realmlist
  ADD COLUMN realmbuilds varchar(64) NOT NULL default '' AFTER population;
