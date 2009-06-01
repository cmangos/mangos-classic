ALTER TABLE character_db_version CHANGE COLUMN required_032_7059_02_characters_pet_spell required_041_7903_01_characters_character_pet bit;

UPDATE character_pet
  SET abdata = CONCAT(REPLACE(TRIM(abdata),'  ',' '),' ');

UPDATE character_pet
  SET abdata = SUBSTRING_INDEX(SUBSTRING_INDEX(abdata,' ',(10-3)*2),' ',-(10-3-3)*2)
  WHERE length(SUBSTRING_INDEX(abdata, ' ', 20)) < length(abdata) and length(SUBSTRING_INDEX(abdata, ' ', 21)) >= length(abdata);

