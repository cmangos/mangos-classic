ALTER TABLE db_version CHANGE COLUMN required_z2749_01_mangos_dbscript_priority_miliseconds required_z2750_01_mangos_graveyard_map_links bit;

-- Rename ghost_zone to ghost_loc because ghost_loc can now also refer to a map, not just an area
-- Add link_kind to indicate what `ghost_loc` refers to.
-- There are two
-- kinds:
--   0 --> `ghost_loc` refers to an area
--   1 --> `ghost_loc` refers to a map

ALTER TABLE `game_graveyard_zone`
    CHANGE `ghost_zone` `ghost_loc` MEDIUMINT UNSIGNED NOT NULL,
    ADD COLUMN `link_kind` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `ghost_loc`,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY(`id`, `ghost_loc`, `link_kind`);
