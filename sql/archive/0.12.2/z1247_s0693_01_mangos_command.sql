ALTER TABLE db_version CHANGE COLUMN required_z1240_s0671_01_mangos_spell_proc_event required_z1247_s0693_01_mangos_command bit;

DELETE FROM command WHERE name IN ('send mass mail');

INSERT INTO command (name, security, help) VALUES
('send mass mail',3,'Syntax: .send mass mail #racemask|$racename|alliance|horde|all "#subject" "#text"\r\n\r\nSend a mail to players. Subject and mail text must be in "".');
