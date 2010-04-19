ALTER TABLE db_version CHANGE COLUMN required_z0405_101_01_mangos_mangos_string required_z0447_107_01_mangos_gossip_menu_option bit;

UPDATE gossip_menu_option SET option_icon=0 WHERE menu_id=0 AND option_id=16;
