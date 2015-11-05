ALTER TABLE db_version CHANGE COLUMN required_z0095_090_01_mangos_player_xp_for_level required_z0096_xxx_01_mangos bit;


-- blood fury ( warrior ) fix

UPDATE playercreateinfo_spell SET `Spell` = '20572' WHERE `playercreateinfo_spell`.`race` =2 AND `playercreateinfo_spell`.`class` =7 AND `playercreateinfo_spell`.`Spell` =33697 LIMIT 1 ;

UPDATE playercreateinfo_spell SET `Spell` = '20572' WHERE `playercreateinfo_spell`.`race` =2 AND `playercreateinfo_spell`.`class` =9 AND `playercreateinfo_spell`.`Spell` =33702 LIMIT 1 ;

UPDATE playercreateinfo_action SET `action` = '20572' WHERE `playercreateinfo_action`.`race` =2 AND `playercreateinfo_action`.`class` =9 AND `playercreateinfo_action`.`button` =3 LIMIT 1 ;

UPDATE playercreateinfo_action SET `action` = '20572' WHERE `playercreateinfo_action`.`race` =2 AND `playercreateinfo_action`.`class` =7 AND `playercreateinfo_action`.`button` =3 LIMIT 1 ;

-- this pet doesn't exist in pre-bc

DELETE FROM spell_pet_auras WHERE pet = 17252;

-- TBC spells
DELETE FROM playercreateinfo_spell WHERE spell IN ( 32215, 34082, 33948, 34123, 40121 );
