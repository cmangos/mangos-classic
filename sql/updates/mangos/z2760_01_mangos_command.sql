ALTER TABLE db_version CHANGE COLUMN required_z2759_01_mangos_gameobject_template required_z2760_01_mangos_command bit;

DELETE FROM command WHERE name IN ('movegens','cooldown', 'modify honor');

REPLACE INTO command VALUES
('cooldown list',3,'Syntax: .cooldown list  Active cooldown from selected unit.'),
('cooldown clear',3,'Syntax: .cooldown clear [spell id] Remove cooldown from selected unit.'),
('cooldown clearclientside',3,'Syntax: .cooldown clearclientside  Clear all cooldown client side only.'),
('movement movegens',3,'Syntax: .movement movegens  Show movement generators stack for selected creature or player.'),
('movement cometome',3,'Syntax: .movement cometome  Move selected creature to you.'),
('movement movespeed',3,'Syntax: .movement movespeed  Show speed of selected creature.'),
('modify honor',1,'Syntax: .modify honor $field $value\r\n\r\nAdd $value to an honor $field (it\'s just for debug , values won\'t be saved at player logout)\r\nField list: points / rank / todaykills / yesterdaykills / yesterdayhonor /\r\nthisweekkills / thisweekhonor / lastweekkills / lastweekhonor /\r\nlastweekstanding / lifetimedishonorablekills / lifetimehonorablekills ');
