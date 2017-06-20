ALTER TABLE db_version CHANGE COLUMN required_z2697_02_mangos_gameobjects required_z2698_01_mangos_gossip_menu_option bit;

DELETE FROM gossip_menu_option WHERE menu_id=0 AND id=16;
INSERT INTO gossip_menu_option VALUES(0,16,0,'GOSSIP_OPTION_BOT',99,1,0,0,0,0,0,NULL,0);
