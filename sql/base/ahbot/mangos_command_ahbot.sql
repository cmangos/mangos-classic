DELETE FROM command WHERE name LIKE 'ahbot%';
INSERT INTO command VALUES
('ahbot rebuild',3,'Syntax: .ahbot rebuild [all]\r\n\r\nExpire all auctions by ahbot except those bidded on by a player. Bidded auctions can be forced expired by using the \"all\" option. AHBot will re-fill auctions using current settings.'),
('ahbot reload',3,'Syntax: .ahbot reload\r\n\r\nReload AHBot settings from configuration file.'),
('ahbot status',3,'Syntax: .ahbot status\r\n\r\nShow current amount of items added to the auction house by AHBot.'),
('ahbot item',3,'Syntax: .ahbot item #itemid [$itemvalue [$addchance [$minamount [$maxamount]]]] [reset]\r\n\r\nShow/modify AHBot item. Setting $itemvalue to 0 bans item. Setting $addchance greater than 0 (0-100, default 0) overrides normal loot sources in favor of a fixed add chance. Min/max amount defaults to item stack size. Parameter \"reset\" resets item configuration.');
