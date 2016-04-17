DROP TABLE IF EXISTS `account_note`;
CREATE TABLE `account_note` (
    id INT(11) UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
    accountId INT(11) UNSIGNED NOT NULL,
    noteTime timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
    accountNote varchar(255) NOT NULL
);
