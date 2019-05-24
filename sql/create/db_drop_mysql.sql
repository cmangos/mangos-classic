/* Delete default user and its privileges (require MYSQL 5.7, MARIADB 10.1.3) */
DROP USER IF EXISTS `mangos`@`localhost`;

/* Delete default tables */
DROP DATABASE IF EXISTS `classicmangos` ;
DROP DATABASE IF EXISTS `classiccharacters` ;
DROP DATABASE IF EXISTS `classicrealmd` ;
