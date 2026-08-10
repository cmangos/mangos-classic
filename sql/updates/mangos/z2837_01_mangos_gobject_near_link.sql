ALTER TABLE db_version CHANGE COLUMN required_z2836_01_mangos_cls_rework required_z2837_01_mangos_gobject_near_link bit;

UPDATE mangos_string SET content_default='%d%s, Entry %d - |cffffffff|Hgameobject:%d:%d|h[%s X:%f Y:%f Z:%f MapId:%d]|h|r SpawnGroup:%u' WHERE entry=517;
