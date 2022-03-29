ALTER TABLE db_version CHANGE COLUMN required_z2776_01_mangos_creature_immunity required_z2777_01_mangos_corpse_decay bit;

ALTER TABLE creature_template ADD COLUMN CorpseDecay INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Time before corpse despawns' AFTER VisibilityDistanceType;

