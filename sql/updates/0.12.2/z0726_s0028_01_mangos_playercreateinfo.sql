ALTER TABLE db_version CHANGE COLUMN required_z0718_s0020_01_mangos_playercreateinfo required_z0726_s0028_01_mangos_playercreateinfo bit;

UPDATE playercreateinfo SET orientation= 5.696318 WHERE race=4;
UPDATE playercreateinfo SET orientation= 6.177156 WHERE race=3;
UPDATE playercreateinfo SET orientation= 2.70526 WHERE race=5;
