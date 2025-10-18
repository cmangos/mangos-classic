#include "playerbot/strategy/Action.h"
#include "WorldBuffAction.h"
#include "playerbot/AiFactory.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "GameEvents/GameEventMgr.h"

using namespace ai;

bool WorldBuffAction::Execute(Event& event)
{    
    std::string text = event.getParam();

    for (auto& wb : NeedWorldBuffs(bot))
    {
        ai->AddAura(bot, wb);
    }

    return false;
}

std::vector<uint32> WorldBuffAction::NeedWorldBuffs(Unit* unit)
{
    std::vector<uint32> retVec;

    if (sPlayerbotAIConfig.worldBuffs.empty())
        return retVec;

    Player* plr = static_cast<Player*>(unit);
   
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    uint32 factionId = PlayerbotAI::GetFactionReaction(unit->GetFactionTemplateEntry(), humanFaction) >= REP_NEUTRAL ? 1 : 2;

    for (auto& wb : sPlayerbotAIConfig.worldBuffs)
    {
        if (wb.factionId != 0 && wb.factionId != factionId)
            continue;

        if (wb.classId != 0 && wb.classId != unit->getClass())
            continue;

        if (plr && wb.specId != 0 && wb.specId != (AiFactory::GetPlayerSpecTab(plr) + 1))
            continue;

        if (wb.minLevel != 0 && wb.minLevel > unit->GetLevel())
            continue;

        if (wb.maxLevel != 0 && wb.maxLevel < unit->GetLevel())
            continue;

        if (wb.eventId != 0 && !sGameEventMgr.IsActiveEvent(wb.eventId))
            continue;

        if (unit->HasAura(wb.spellId))
            continue;

        retVec.push_back(wb.spellId);
    }

    return retVec;
}

