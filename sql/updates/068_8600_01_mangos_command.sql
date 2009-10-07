DELETE FROM command where name='instance unbind';

INSERT INTO `command` VALUES
('instance unbind',3,'Syntax: .instance unbind all\r\n  All of the selected
player\'s binds will be cleared.\r\n.instance unbind #mapid\r\n Only the
specified #mapid instance will be cleared.');
