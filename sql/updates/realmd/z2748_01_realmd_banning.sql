ALTER TABLE realmd_db_version CHANGE COLUMN required_z2743_00_realmd_account_drop_sha required_z2748_01_realmd_banning bit;

ALTER TABLE account_banned CHANGE id account_id int(11);
ALTER TABLE account_banned DROP PRIMARY KEY;
ALTER TABLE account_banned CHANGE bandate banned_at bigint(40);
ALTER TABLE account_banned CHANGE unbandate expires_at bigint(40);
ALTER TABLE account_banned CHANGE banreason reason varchar(255);
ALTER TABLE account_banned CHANGE bannedby banned_by varchar(50);
ALTER TABLE account_banned ADD COLUMN unbanned_at bigint(40) NOT NULL DEFAULT '0' AFTER banned_by;
ALTER TABLE account_banned ADD COLUMN unbanned_by varchar(50) DEFAULT NULL AFTER unbanned_at;
ALTER TABLE account_banned ADD COLUMN id int(11) PRIMARY KEY AUTO_INCREMENT FIRST;

ALTER TABLE ip_banned CHANGE bandate banned_at bigint(40);
ALTER TABLE ip_banned CHANGE unbandate expires_at bigint(40);
ALTER TABLE ip_banned CHANGE bannedby banned_by varchar(50);
ALTER TABLE ip_banned CHANGE banreason reason varchar(255);


