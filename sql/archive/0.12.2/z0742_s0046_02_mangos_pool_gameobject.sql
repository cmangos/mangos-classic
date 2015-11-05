ALTER TABLE db_version CHANGE COLUMN required_z0742_s0046_01_mangos_pool_creature required_z0742_s0046_02_mangos_pool_gameobject bit;

DROP TABLE IF EXISTS pool_gameobject_temp;
CREATE TABLE pool_gameobject_temp
SELECT guid, pool_entry, chance, description FROM pool_gameobject GROUP BY guid;

ALTER TABLE pool_gameobject_temp
  ADD PRIMARY KEY  (guid),
  ADD INDEX pool_idx (pool_entry);

DROP TABLE IF EXISTS pool_gameobject;
RENAME TABLE pool_gameobject_temp TO pool_gameobject;
