ALTER TABLE db_version CHANGE COLUMN required_z1018_s0391_02_mangos_creature_template required_z1018_xxxxx_03_mangos_command bit;

DELETE FROM command WHERE name IN ('npc additem');

INSERT INTO command (name, security, help) VALUES
('npc additem',2,'Syntax: .npc additem #itemId <#maxcount><#incrtime>r\r\n\r\nAdd item #itemid to item list of selected vendor. Also optionally set max count item in vendor item list and time to item count restoring.');

