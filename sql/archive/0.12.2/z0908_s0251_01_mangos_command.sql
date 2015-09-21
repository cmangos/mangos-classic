ALTER TABLE db_version CHANGE COLUMN required_z0889_s0224_04_mangos_gameobject_respawn required_z0908_s0251_01_mangos_command bit;

DELETE FROM command WHERE name IN ('account characters');

INSERT INTO command (name, security, help) VALUES
('account characters',3,'Syntax: .account characters [#accountId|$accountName]\r\n\r\nShow list all characters for account selected by provided #accountId or $accountName, or for selected player in game.');

