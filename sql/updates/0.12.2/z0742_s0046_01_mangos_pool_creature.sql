ALTER TABLE db_version CHANGE COLUMN required_z0738_s0041_02_mangos_command required_z0742_s0046_01_mangos_pool_creature bit;

DROP TABLE IF EXISTS pool_creature_temp;
CREATE TABLE pool_creature_temp
SELECT guid, pool_entry, chance, description FROM pool_creature GROUP BY guid;

ALTER TABLE pool_creature_temp
  ADD PRIMARY KEY  (guid),
  ADD INDEX pool_idx (pool_entry);

DROP TABLE IF EXISTS pool_creature;
RENAME TABLE pool_creature_temp TO pool_creature;
