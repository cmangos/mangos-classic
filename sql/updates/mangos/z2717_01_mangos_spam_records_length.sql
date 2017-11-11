ALTER TABLE db_version CHANGE COLUMN required_z2715_01_mangos_spam_records required_z2717_01_mangos_spam_records_length bit;

ALTER TABLE spam_records MODIFY record VARCHAR(512) NOT NULL;

