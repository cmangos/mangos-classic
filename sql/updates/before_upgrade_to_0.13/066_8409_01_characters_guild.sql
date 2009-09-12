ALTER TABLE character_db_version CHANGE COLUMN required_066_8409_01_characters_guild required_066_8402_02_characters_guild_bank_eventlog bit;


-- Change createdate column type from datetime to bigint(20)

-- add temporary column
ALTER TABLE guild ADD COLUMN created_temp datetime default NULL;
-- update temporary columns data
UPDATE guild SET created_temp = FROM_UNIXTIME(createdate);
-- drop current column
ALTER TABLE guild DROP COLUMN createdate;
-- create new column with correct type
ALTER TABLE guild ADD COLUMN createdate datetime default NULL AFTER motd;
-- copy data to new column
UPDATE guild set createdate = created_temp;
-- remove old column
ALTER TABLE guild DROP COLUMN created_temp;
