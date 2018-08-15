REVOKE ALL PRIVILEGES ON * . * FROM 'mangos'@'localhost';

REVOKE ALL PRIVILEGES ON `classicmangos` . * FROM 'mangos'@'localhost';

REVOKE GRANT OPTION ON `classicmangos` . * FROM 'mangos'@'localhost';

REVOKE ALL PRIVILEGES ON `classiccharacters` . * FROM 'mangos'@'localhost';

REVOKE GRANT OPTION ON `classiccharacters` . * FROM 'mangos'@'localhost';

REVOKE ALL PRIVILEGES ON `classicrealmd` . * FROM 'mangos'@'localhost';

REVOKE GRANT OPTION ON `classicrealmd` . * FROM 'mangos'@'localhost';

DELETE FROM `user` WHERE CONVERT( User USING utf8 ) = CONVERT( 'mangos' USING utf8 ) AND CONVERT( Host USING utf8 ) = CONVERT( 'localhost' USING utf8 ) ;

DROP DATABASE IF EXISTS `classicmangos` ;

DROP DATABASE IF EXISTS `classiccharacters` ;

DROP DATABASE IF EXISTS `classicrealmd` ;
