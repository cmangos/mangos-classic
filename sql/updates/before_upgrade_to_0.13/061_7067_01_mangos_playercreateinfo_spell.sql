ALTER TABLE db_version CHANGE COLUMN required_061_7067_01_mangos_playercreateinfo_spell required_058_7544_01_mangos_uptime bit;

ALTER TABLE playercreateinfo_spell
  ADD COLUMN `Active` tinyint(3) unsigned NOT NULL default '1' AFTER Note;
