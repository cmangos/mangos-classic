UPDATE `creature_template` SET `ScriptName` = 'npc_tervosh' WHERE `Entry` = '4967';

INSERT INTO `script_texts` ( `entry`, `content_default`,  `sound`, `type`, `emote`, `language`, `comment` ) 
VALUES 
	('-1999926', 'Farewell. We shall speak again. I\'m sure.', '0', '0', '0', '7', 'tervosh SAY_FARWELL');



UPDATE `spell_template` SET `EffectImplicitTargetA1` = '8' WHERE `Id` = '7143'; 

INSERT INTO `spell_script_target` (`entry`, `type`, `targetEntry`, `inverseEffectMask`)
VALUES 
	('7143', '1', '4968', '0'),
	('7143', '1', '4967', '0'),
	('7143', '1', '4965', '0');        

