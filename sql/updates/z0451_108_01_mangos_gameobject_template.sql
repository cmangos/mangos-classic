ALTER TABLE db_version CHANGE COLUMN required_z0447_107_01_mangos_gossip_menu_option required_z0451_108_01_mangos_gameobject_template bit;

ALTER TABLE gameobject_template ADD COLUMN mingold MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER data23;
ALTER TABLE gameobject_template ADD COLUMN maxgold MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0' AFTER mingold;
