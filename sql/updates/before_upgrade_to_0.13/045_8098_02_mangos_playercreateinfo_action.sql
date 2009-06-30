ALTER TABLE db_version CHANGE COLUMN required_045_8098_02_mangos_playercreateinfo_action required_043_7945_01_mangos_quest_template bit;

ALTER TABLE playercreateinfo_action
  ADD COLUMN misc int(11) unsigned NOT NULL default '0';
