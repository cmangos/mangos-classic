ALTER TABLE db_version CHANGE COLUMN required_z2187_s1820_12269_09_mangos_pickpocketing_loot_template required_z2187_s1820_12269_10_mangos_gossip_menu bit;

ALTER TABLE gossip_menu DROP COLUMN cond_2_val_2, DROP COLUMN cond_2_val_1, DROP COLUMN cond_2,
                        DROP COLUMN cond_1_val_2, DROP COLUMN cond_1_val_1, DROP COLUMN cond_1;
