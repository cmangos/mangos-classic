ALTER TABLE db_version CHANGE COLUMN required_z0379_098_01_mangos_gameobject required_z0385_099_01_mangos_item_template bit;

alter table item_template
  add column NonConsumable tinyint(1) UNSIGNED DEFAULT '0' NOT NULL after Duration;
