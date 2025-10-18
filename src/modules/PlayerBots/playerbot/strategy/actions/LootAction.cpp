
#include "playerbot/playerbot.h"
#include "LootAction.h"

#include "playerbot/LootObjectStack.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/LootStrategyValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/SharedValueContext.h"


using namespace ai;

bool LootAction::Execute(Event& event)
{
    if (!AI_VALUE(bool, "has available loot"))
        return false;

    LootObject prevLoot = AI_VALUE(LootObject, "loot target");
    LootObject const& lootObject = AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig.lootDistance);

    if (!prevLoot.IsEmpty() && prevLoot.guid != lootObject.guid)
    {
        WorldPacket packet(CMSG_LOOT_RELEASE, 8);
        packet << prevLoot.guid;
        bot->GetSession()->HandleLootReleaseOpcode(packet);
    }

    context->GetValue<LootObject>("loot target")->Set(lootObject);
    return true;
}

enum ProfessionSpells
{
    ALCHEMY                      = 2259,
    BLACKSMITHING                = 2018,
    COOKING                      = 2550,
    ENCHANTING                   = 7411,
    ENGINEERING                  = 49383,
    FIRST_AID                    = 3273,
    FISHING                      = 7620,
    HERB_GATHERING               = 2366,
    INSCRIPTION                  = 45357,
    JEWELCRAFTING                = 25229,
    MINING                       = 2575,
    SKINNING                     = 8613,
    TAILORING                    = 3908
};

bool OpenLootAction::Execute(Event& event)
{
    LootObject lootObject = AI_VALUE(LootObject, "loot target");
    bool result = DoLoot(lootObject);
    if (result)
    {
        AI_VALUE(LootObjectStack*, "available loot")->Remove(lootObject.guid);
        context->GetValue<LootObject>("loot target")->Set(LootObject());
    }
    return result;
}

bool OpenLootAction::DoLoot(LootObject& lootObject)
{
    if (lootObject.IsEmpty())
        return false;

    Creature* creature = ai->GetCreature(lootObject.guid);
    if (creature && sServerFacade.GetDistance2d(bot, creature) > INTERACTION_DISTANCE)
        return false;

    if (creature && creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE) && !creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        if (!lootObject.IsLootPossible(bot)) //Clear loot if bot can't loot it.
            return true;

        WorldPacket packet(CMSG_LOOT, 8);
        packet << lootObject.guid;
        bot->GetSession()->HandleLootOpcode(packet);
        SetDuration(sPlayerbotAIConfig.lootDelay);

        if (bot->isRealPlayer())
        {
            WorldPacket data(SMSG_EMOTE, 4 + 8);
            data << uint32(EMOTE_ONESHOT_LOOT);
            data << bot->GetObjectGuid();
            bot->GetSession()->SendPacket(data);
        }

        return true;
    }

    if (creature)
    {
        SkillType skill = creature->GetCreatureInfo()->GetRequiredLootSkill();
        if (!CanOpenLock(skill, lootObject.reqSkillValue))
            return false;

        switch (skill)
        {
        case SKILL_ENGINEERING:
            return ai->HasSkill(SKILL_ENGINEERING) ? ai->CastSpell(ENGINEERING, creature) : false;
        case SKILL_HERBALISM:
            return ai->HasSkill(SKILL_HERBALISM) ? ai->CastSpell(32605, creature) : false;
        case SKILL_MINING:
            return ai->HasSkill(SKILL_MINING) ? ai->CastSpell(32606, creature) : false;
        default:
            return ai->HasSkill(SKILL_SKINNING) ? ai->CastSpell(SKINNING, creature) : false;
        }
    }

    GameObject* go = ai->GetGameObject(lootObject.guid);
    if (go && sServerFacade.GetDistance2d(bot, go) > INTERACTION_DISTANCE)
        return false;

    if (go && (go->IsInUse() || go->GetGoState() == GO_STATE_ACTIVE))
        return false;

    if (lootObject.skillId == SKILL_MINING)
        return ai->HasSkill(SKILL_MINING) ? ai->CastSpell(MINING, bot) : false;

    if (lootObject.skillId == SKILL_HERBALISM)
    {
        // herb-like quest objects
        bool isForQuest = false;
        if (go && sObjectMgr.IsGameObjectForQuests(lootObject.guid.GetEntry()))
        {
            if (go->ActivateToQuest(bot))
            {
                std::list<uint32> lootItems = GAI_VALUE2(std::list<uint32>, "entry loot list", -1*int32(go->GetEntry()));
                isForQuest = !lootItems.empty() || go->GetLootState() != GO_READY;
            }
        }

        if (!isForQuest)
        {
            return ai->HasSkill(SKILL_HERBALISM) ? ai->CastSpell(HERB_GATHERING, bot) : false;
        }
    }

    uint32 spellId = GetOpeningSpell(lootObject);
    if (!spellId)
        return false;

    if (!lootObject.IsLootPossible(bot)) //Clear loot if bot can't loot it.
        return true;

    //Keys need to use the key 
    if (spellId == sPlayerbotAIConfig.openGoSpell && go && lootObject.reqItem && bot->HasItemCount(lootObject.reqItem,1,false))
    {
        return ai->DoSpecificAction("use", Event("do loot", chat->formatQItem(lootObject.reqItem) + " " + chat->formatGameobject(go)));
    }

    return ai->CastSpell(spellId, bot);
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject)
{
    GameObject* go = ai->GetGameObject(lootObject.guid);
    if (go && sServerFacade.isSpawned(go))
        return GetOpeningSpell(lootObject, go);

    return 0;
}

uint32 OpenLootAction::GetOpeningSpell(LootObject& lootObject, GameObject* go)
{
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

		if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
			continue;

		if (spellId == MINING || spellId == HERB_GATHERING)
			continue;

		const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
		if (!pSpellInfo)
			continue;

        if (CanOpenLock(lootObject, pSpellInfo, go))
            return spellId;
    }

    for (uint32 spellId = 0; spellId < sServerFacade.GetSpellInfoRows(); spellId++)
    {
        if (spellId == MINING || spellId == HERB_GATHERING)
            continue;

		const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
		if (!pSpellInfo)
            continue;

        if (CanOpenLock(lootObject, pSpellInfo, go))
            return spellId;
    }

    return sPlayerbotAIConfig.openGoSpell;
}

bool OpenLootAction::CanOpenLock(LootObject& lootObject, const SpellEntry* pSpellInfo, GameObject* go)
{
    for (int effIndex = 0; effIndex <= EFFECT_INDEX_2; effIndex++)
    {
        if (pSpellInfo->Effect[effIndex] != SPELL_EFFECT_OPEN_LOCK && pSpellInfo->Effect[effIndex] != SPELL_EFFECT_SKINNING)
            return false;

        uint32 lockId = go->GetGOInfo()->GetLockId();
        if (!lockId)
            return false;

        LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);
        if (!lockInfo)
            return false;

        bool reqKey = false;                                    // some locks not have reqs

        for(int j = 0; j < 8; ++j)
        {
            switch(lockInfo->Type[j])
            {
            /*
            case LOCK_KEY_ITEM:
                return true;
            */
            case LOCK_KEY_SKILL:
                {
                    if(uint32(pSpellInfo->EffectMiscValue[effIndex]) != lockInfo->Index[j])
                        continue;

                    uint32 skillId = SkillByLockType(LockType(lockInfo->Index[j]));
                    if (skillId == SKILL_NONE)
                        return true;

                    if (CanOpenLock(skillId, lockInfo->Skill[j]))
                        return true;
                }
            }
        }
    }

    return false;
}

bool OpenLootAction::CanOpenLock(uint32 skillId, uint32 reqSkillValue)
{
    uint32 skillValue = bot->GetSkillValue(skillId);
    return skillValue >= reqSkillValue || !reqSkillValue;
}

bool StoreLootAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p(event.getPacket()); // (8+1+4+1+1+4+4+4+4+4+1)
    ObjectGuid guid;
    uint8 loot_type;
    uint32 gold = 0;
    uint8 items = 0;

    p.rpos(0);
    p >> guid;      // 8 corpse guid
    p >> loot_type; // 1 loot type

    if (p.size() > 10)
    {
        p >> gold;      // 4 money on corpse
        p >> items;     // 1 number of items on corpse
    }

    bot->SetLootGuid(guid);

    if (gold > 0)
    {
        WorldPacket packet(CMSG_LOOT_MONEY, 0);
        bot->GetSession()->HandleLootMoneyOpcode(packet);
    }

    for (uint8 i = 0; i < items; ++i)
    {
        uint32 itemid;
        uint32 randomPropertyId;
        uint32 itemcount;
        uint8 lootslot_type;
        uint8 itemindex;
        bool grab = false;

        p >> itemindex;
        p >> itemid;
        p >> itemcount;
        p.read_skip<uint32>();  // display id
        p.read_skip<uint32>();  // randomSuffix
        p >> randomPropertyId;  // randomPropertyId
        p >> lootslot_type;     // 0 = can get, 1 = look only, 2 = master get

        ItemQualifier itemQualifier(itemid, ((int32)randomPropertyId));

		if (lootslot_type != LOOT_SLOT_NORMAL
#ifndef MANGOSBOT_ZERO
		        && lootslot_type != LOOT_SLOT_OWNER
#endif
            )
			continue;

        if (loot_type != LOOT_SKINNING && !IsLootAllowed(itemQualifier, ai))
            continue;

        if (AI_VALUE2(uint32, "stack space for item", itemid) < itemcount)
            continue;

        ItemPrototype const *proto = sItemStorage.LookupEntry<ItemPrototype>(itemid);
        if (!proto)
            continue;

        Loot* loot = sLootMgr.GetLoot(bot);

        if (!loot)
            continue;

        LootItem* lootItem = loot->GetLootItemInSlot(itemindex);

        if (!lootItem)
            continue;

        //have no right to loot
        if (lootItem->isBlocked || lootItem->GetSlotTypeForSharedLoot(bot, loot) == MAX_LOOT_SLOT_TYPE)
            continue;

        Player* master = ai->GetMaster();
        if (sRandomPlayerbotMgr.IsRandomBot(bot) && master)
        {
            uint32 price = itemcount * ItemUsageValue::GetBotBuyPrice(proto, bot) + gold;
            if (price)
                sRandomPlayerbotMgr.AddTradeDiscount(bot, master, price);
        }

        WorldPacket packet(CMSG_AUTOSTORE_LOOT_ITEM, 1);
        packet << itemindex;
        bot->GetSession()->HandleAutostoreLootItemOpcode(packet);

        if (proto->Quality > ITEM_QUALITY_NORMAL && !urand(0, 50) && ai->HasStrategy("emote", BotState::BOT_STATE_NON_COMBAT)) ai->PlayEmote(TEXTEMOTE_CHEER);
        if (proto->Quality >= ITEM_QUALITY_RARE && !urand(0, 1) && ai->HasStrategy("emote", BotState::BOT_STATE_NON_COMBAT)) ai->PlayEmote(TEXTEMOTE_CHEER);

        if (requester && (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT) || (requester->GetMapId() != bot->GetMapId() || WorldPosition(requester).sqDistance2d(bot) > (sPlayerbotAIConfig.sightDistance * sPlayerbotAIConfig.sightDistance))))
        {
            std::map<std::string, std::string> args;
            args["%item"] = chat->formatItem(itemQualifier);
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("loot_command", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }

        sPlayerbotAIConfig.logEvent(ai, "StoreLootAction", proto->Name1, std::to_string(proto->ItemId));

        BroadcastHelper::BroadcastLootingItem(ai, bot, proto, itemQualifier);
    }

    AI_VALUE(LootObjectStack*, "available loot")->Remove(guid);
    RESET_AI_VALUE(LootObject, "loot target");
    RESET_AI_VALUE2(bool, "should loot object", std::to_string(guid.GetRawValue()));

    // release loot
    WorldPacket packet(CMSG_LOOT_RELEASE, 8);
    packet << guid;
    bot->GetSession()->HandleLootReleaseOpcode(packet);

    ai->AccelerateRespawn(guid);

    return true;
}

bool StoreLootAction::IsLootAllowed(ItemQualifier& itemQualifier, PlayerbotAI *ai)
{
    AiObjectContext *context = ai->GetAiObjectContext();
    
    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemQualifier.GetId());
    if (!proto)
        return false;

    std::set<uint32>& lootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    if (lootItems.find(itemQualifier.GetId()) != lootItems.end())
        return true;

    std::set<uint32>& skipItems = AI_VALUE(std::set<uint32>&, "skip loot list");
    if (skipItems.find(itemQualifier.GetId()) != skipItems.end())
        return false;

    uint32 max = proto->MaxCount;
    if (max > 0 && ai->GetBot()->HasItemCount(itemQualifier.GetId(), max, true))
        return false;

    if (proto->StartQuest)
    {
        if (sPlayerbotAIConfig.syncQuestWithPlayer)
            return false; //Quest is autocomplete for the bot so no item needed.
        else
            return true;
    }

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = ai->GetBot()->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr.GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (int i = 0; i < 4; i++)
        {
            if (quest->ReqItemId[i] == itemQualifier.GetId())
            {
                if (ai->GetMaster() && sPlayerbotAIConfig.syncQuestWithPlayer)
                    return false; //Quest is autocomplete for the bot so no item needed.

                if (AI_VALUE2(uint32, "item count", proto->Name1) >= quest->ReqItemCount[i])
                    return false;
            }
        }
    }

    //if (proto->Bonding == BIND_QUEST_ITEM ||  //Still testing if it works ok without these lines.
    //    proto->Bonding == BIND_QUEST_ITEM1 || //Eventually this has to be removed.
    //    proto->Class == ITEM_CLASS_QUEST)
    //{

    bool canLoot = LootStrategyValue::CanLoot(itemQualifier, ai);

    //if (canLoot && proto->Bonding == BIND_WHEN_PICKED_UP && ai->HasActivePlayerMaster())
    //    canLoot = sPlayerbotAIConfig.IsInRandomAccountList(sObjectMgr.GetPlayerAccountIdByGUID(ai->GetBot()->GetObjectGuid()));

    return canLoot;
}

bool ReleaseLootAction::Execute(Event& event)
{
    std::list<ObjectGuid> gos = context->GetValue<std::list<ObjectGuid> >("nearest game objects no los")->Get();
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
    {
        WorldPacket packet(CMSG_LOOT_RELEASE, 8);
        packet << *i;
        bot->GetSession()->HandleLootReleaseOpcode(packet);
    }

    std::list<ObjectGuid> corpses = context->GetValue<std::list<ObjectGuid> >("nearest corpses")->Get();
    for (std::list<ObjectGuid>::iterator i = corpses.begin(); i != corpses.end(); i++)
    {
        WorldPacket packet(CMSG_LOOT_RELEASE, 8);
        packet << *i;
        bot->GetSession()->HandleLootReleaseOpcode(packet);
    }

    return true;
}
