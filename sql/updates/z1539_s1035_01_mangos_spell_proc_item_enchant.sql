ALTER TABLE db_version CHANGE COLUMN required_z1529_s1024_01_mangos_mangos_string required_z1539_s1035_01_mangos_spell_proc_item_enchant bit;

DELETE FROM spell_proc_item_enchant  WHERE entry IN (13897, 20004, 20005);
INSERT INTO spell_proc_item_enchant VALUES
(13897, 6.0), -- Enchant Weapon - Fiery Weapon
(20004, 6.0), -- Enchant Weapon - Lifestealing
(20005, 1.6); -- Enchant Weapon - Icy Chill
