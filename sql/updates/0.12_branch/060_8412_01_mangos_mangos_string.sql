DELETE FROM mangos_string WHERE entry IN(512,513);
INSERT INTO mangos_string VALUES
(512,'%d - |cffffffff|Hitem:%d:0:0:0:0:0:0:0|h[%s]|h|r ',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL),
(513,'%d - |cffffffff|Hquest:%d:%d|h[%s]|h|r %s',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
