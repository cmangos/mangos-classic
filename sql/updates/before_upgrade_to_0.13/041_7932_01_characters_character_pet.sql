ALTER TABLE character_db_version CHANGE COLUMN required_041_7903_01_characters_character_pet required_032_7059_02_characters_pet_spell bit;

UPDATE character_pet
  SET abdata = CONCAT(REPLACE(TRIM(abdata),'  ',' '),' ');

UPDATE character_pet
  SET abdata = CONCAT('0 0 0 0 0 0 ',abdata,'0 0 0 0 0 0 ')
  WHERE length(SUBSTRING_INDEX(abdata, ' ', 8)) < length(abdata) and length(SUBSTRING_INDEX(abdata, ' ', 9)) >= length(abdata);

