DELETE FROM command WHERE name = 'account note';
INSERT INTO command (name, security, help) VALUES ('account note', 1, 'Syntax: .account note [accountName] [accountNote]');
