ALTER TABLE db_version CHANGE COLUMN required_z0775_s0083_01_mangos_mangos_string required_z0782_xxxxx_01_mangos_mangos_string bit;

UPDATE mangos_string SET entry = 1400 + (entry - 349) WHERE entry >= 349 AND entry <= 388;
