ALTER TABLE db_version CHANGE COLUMN required_z0101_xxx_01_mangos_item_template required_z0124_xxx_02_mangos bit;


ALTER TABLE game_event DROP COLUMN holiday;
ALTER TABLE npc_option DROP COLUMN box_money;
ALTER TABLE npc_vendor DROP COLUMN ExtendedCost;
ALTER TABLE spell_facing CHANGE COLUMN entry  entry int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell ID', ADD PRIMARY KEY (entry);
ALTER TABLE spell_facing CHANGE COLUMN facingcasterflag facingcasterflag tinyint(1) NOT NULL DEFAULT '1' COMMENT 'flag for facing state, usually 1';


UPDATE mangos_string SET content_default = 'Item \'%i\' \'%s\' added to list with maxcount \'%i\' and incrtime \'%i\' ' WHERE entry = 206;
