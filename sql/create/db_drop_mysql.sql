/* Delete default user and its privileges (require MYSQL 5.7, MARIADB 10.1.3) */
DROP USER IF EXISTS `mangos`@`localhost`;

/* Delete default tables */
DROP DATABASE IF EXISTS `tbcmangos`;
DROP DATABASE IF EXISTS `tbccharacters`;
DROP DATABASE IF EXISTS `tbcrealmd`;
