ALTER TABLE db_version CHANGE COLUMN required_z1854_s1415_11754_01_mangos_mangos_string required_z1873_s1441_11852_01_mangos_gossip_menu bit;

-- Add collumn `script_id` to table `gossip_menu`
ALTER TABLE gossip_menu ADD COLUMN script_id
 mediumint(8) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'script in `gossip_scripts` - will be executed on GossipHello'
 AFTER text_id;

ALTER TABLE gossip_menu
 DROP PRIMARY KEY,
 ADD PRIMARY KEY (entry, text_id, script_id);
