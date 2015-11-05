ALTER TABLE character_db_version CHANGE COLUMN required_z2679_03_characters_guild_member required_z2682_01_characters_pvpstats bit;

ALTER TABLE pvpstats_players CHANGE COLUMN player_guid character_guid int(10) unsigned NOT NULL AFTER battleground_id;
