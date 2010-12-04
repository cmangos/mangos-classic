ALTER TABLE db_version CHANGE COLUMN required_z1022_s0117_01_mangos_creature_ai_scripts required_z1034_s0407_01_mangos_gossip_menu_option bit;

ALTER TABLE gossip_menu_option CHANGE COLUMN action_menu_id action_menu_id MEDIUMINT(8) NOT NULL DEFAULT '0';
