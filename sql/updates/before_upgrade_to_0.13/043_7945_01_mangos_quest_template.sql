ALTER TABLE db_version CHANGE COLUMN required_043_7945_01_mangos_quest_template required_040_7902_02_mangos_pool_gameobject bit;

ALTER TABLE quest_template
  DROP COLUMN `DetailsEmoteDelay1`,
  DROP COLUMN `DetailsEmoteDelay2`,
  DROP COLUMN `DetailsEmoteDelay3`,
  DROP COLUMN `DetailsEmoteDelay4`,
  DROP COLUMN `OfferRewardEmoteDelay1`,
  DROP COLUMN `OfferRewardEmoteDelay2`,
  DROP COLUMN `OfferRewardEmoteDelay3`,
  DROP COLUMN `OfferRewardEmoteDelay4`;
