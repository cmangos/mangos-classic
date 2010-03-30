ALTER TABLE db_version CHANGE COLUMN required_z0385_099_02_mangos_item_template required_z0394_xxx_01_mangos_mangos_string bit;

UPDATE mangos_string SET content_default ='Player: %s - %s (Rank %u)' WHERE entry = 379;
UPDATE mangos_string SET content_default ='Life Time: [Rank Points: |c0000ff00%u|r] [Honorable Kills: |c0000ff00%u|r] [Dishonorable Kills: |c00ff0000%u|r] [Highest Rank %u: %s]' WHERE entry = 384;