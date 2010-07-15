ALTER TABLE db_version CHANGE COLUMN required_z0684_139_01_mangos_mangos_string required_z0718_s0020_01_mangos_playercreateinfo bit;

ALTER TABLE playercreateinfo
  ADD COLUMN orientation float NOT NULL default '0' AFTER position_z;
