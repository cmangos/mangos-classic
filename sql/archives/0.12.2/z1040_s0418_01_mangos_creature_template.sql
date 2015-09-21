ALTER TABLE db_version CHANGE COLUMN required_z1034_s0407_01_mangos_gossip_menu_option required_z1040_s0418_01_mangos_creature_template bit;

ALTER TABLE creature_template ADD COLUMN unit_class tinyint(3) unsigned NOT NULL default '0' AFTER rangeattacktime;

UPDATE creature_template ct
JOIN creature c ON ct.entry=c.id
JOIN creature_addon ad ON c.guid=ad.guid
SET ct.unit_class=(ad.bytes0 & 0x0000FF00) >> 8
WHERE ct.entry=c.id AND ct.unit_class=0;

UPDATE creature_template ct
JOIN creature_template_addon ad ON ct.entry=ad.entry
SET ct.unit_class=(ad.bytes0 & 0x0000FF00) >> 8
WHERE ct.entry=ad.entry AND ct.unit_class=0;
