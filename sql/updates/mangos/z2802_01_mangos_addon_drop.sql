ALTER TABLE db_version CHANGE COLUMN required_z2804_01_mangos_gameobject_data_split required_z2805_01_mangos_addon_drop bit;

ALTER TABLE creature_template_addon DROP COLUMN b2_1_flags;
ALTER TABLE creature_addon DROP COLUMN b2_1_flags;

