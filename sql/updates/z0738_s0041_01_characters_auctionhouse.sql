ALTER TABLE character_db_version CHANGE COLUMN required_z0510_114_01_characters_saved_variables required_z0738_s0041_01_characters_auctionhouse bit;

ALTER TABLE auctionhouse
  ADD COLUMN houseid int(11) unsigned NOT NULL default '0' AFTER id;

UPDATE auctionhouse SET houseid = location;

ALTER TABLE auctionhouse
  DROP COLUMN auctioneerguid,
  DROP COLUMN location;

DROP TABLE IF EXISTS auction;
RENAME TABLE auctionhouse TO auction;
