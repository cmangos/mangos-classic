ALTER TABLE db_version CHANGE COLUMN required_z2688_01_mangos_flametongue required_z2689_01_mangos_item_extraflags bit;

UPDATE item_template SET extraFlags = 1 WHERE extraFlags != 0;