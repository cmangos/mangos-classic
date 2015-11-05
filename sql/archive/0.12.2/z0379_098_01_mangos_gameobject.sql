ALTER TABLE db_version CHANGE COLUMN required_z0367_096_01_mangos_db_script_string required_z0379_098_01_mangos_gameobject bit;

ALTER TABLE gameobject ADD KEY idx_map(map);
ALTER TABLE gameobject ADD KEY idx_id(id);
