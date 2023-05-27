ALTER TABLE db_version CHANGE COLUMN required_z2811_01_mangos_creature_static_flags required_z2812_01_mangos_visibility bit;

UPDATE creature_template SET StaticFlags1=StaticFlags1|0x80000000 WHERE visibilityDistanceType=3;
UPDATE creature_template SET StaticFlags3=StaticFlags3|0x00000400 WHERE visibilityDistanceType=4;
UPDATE creature_template SET StaticFlags3=StaticFlags3|0x00000800 WHERE visibilityDistanceType=5;
ALTER TABLE creature_template DROP COLUMN visibilityDistanceType;
-- tbc did not have gigantic and infinite capability
UPDATE creature_template SET StaticFlags1=StaticFlags1|0x80000000, StaticFlags3=StaticFlags3&~(0x00000400|0x00000800) WHERE StaticFlags3&(0x00000400|0x00000800);


