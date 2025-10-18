#pragma once

#include "WorldPacketTrigger.h"
#include "WithinAreaTrigger.h"

namespace ai
{
    class WorldPacketTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        WorldPacketTriggerContext()
        {
            creators["gossip hello"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "gossip hello"); };
            creators["group invite"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "group invite"); };
            creators["group set leader"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "group set leader"); };
            creators["not enough money"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "not enough money"); };
            creators["not enough reputation"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "not enough reputation"); };
            creators["cannot equip"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "cannot equip"); };
            creators["use game object"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "use game object"); };
            creators["complete quest"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "complete quest"); };
            creators["accept quest"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "accept quest"); };
            creators["confirm quest"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "confirm quest"); };
            creators["quest details"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest details"); };
            creators["quest share"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest share"); };
            creators["loot start roll"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot start roll"); };
            creators["loot roll"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot roll"); };
            creators["resurrect request"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "resurrect request"); };
            creators["area trigger"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "area trigger"); };
            creators["within area trigger"] = [](PlayerbotAI* ai) { return new WithinAreaTrigger(ai); };
            creators["check mount state"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "check mount state"); };
            creators["activate taxi"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "activate taxi"); };
            creators["trade status"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "trade status"); };
            creators["loot response"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot response"); };
            creators["quest update add kill"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add kill"); };
            creators["quest update add item"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add item"); };
            creators["quest update failed"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed"); };
            creators["quest update failed timer"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed timer"); };
            creators["quest update complete"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update complete"); };
            creators["item push result"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "item push result"); };
            creators["party command"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "party command"); };
            creators["taxi done"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "taxi done"); };
            creators["cast failed"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "cast failed"); };
            creators["duel requested"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "duel requested"); };
            creators["ready check"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "ready check"); };
            creators["ready check finished"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "ready check finished"); };
            creators["uninvite"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "uninvite"); };
            creators["uninvite guid"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "uninvite guid"); };
            creators["lfg join"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg join"); };
            creators["lfg proposal"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg proposal"); };
            creators["lfg role check"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg role check"); };
            creators["lfg leave"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg leave"); };
            creators["guild accept"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "guild accept"); };
            creators["petition offer"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "petition offer"); };
            creators["lfg teleport"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg teleport"); };
            creators["inventory change failure"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "inventory change failure"); };
            creators["bg status"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "bg status"); };
            creators["levelup"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "levelup"); };
            creators["xpgain"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "xpgain"); };
            creators["honorgain"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "honorgain"); };
            creators["see spell"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "see spell"); };
            creators["release spirit"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "release spirit"); };
            creators["revive from corpse"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "revive from corpse"); };
            creators["receive emote"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "receive emote"); };
            creators["receive text emote"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "receive text emote"); };
            creators["arena team invite"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "arena team invite"); };
            creators["summon request"] = [](PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "summon request"); };
        }
    };
};
