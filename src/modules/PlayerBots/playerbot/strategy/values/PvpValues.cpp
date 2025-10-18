
#include "playerbot/playerbot.h"
#include "PvpValues.h"
#include "BattleGround/BattleGroundWS.h"
#include "playerbot/ServerFacade.h"
#ifndef MANGOSBOT_ZERO
#include "BattleGround/BattleGroundEY.h"
#endif
#include "playerbot/TravelMgr.h"
#include "SharedValueContext.h"

using namespace ai;

std::list<CreatureDataPair const*> BgMastersValue::Calculate()
{
    BattleGroundTypeId bgTypeId = (BattleGroundTypeId)stoi(qualifier);

    std::vector<uint32> entries;
    std::map<Team, std::map<BattleGroundTypeId, std::list<uint32>>> battleMastersCache = sRandomPlayerbotMgr.getBattleMastersCache();
    entries.insert(entries.end(), battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId].begin(), battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[ALLIANCE][bgTypeId].begin(), battleMastersCache[ALLIANCE][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[HORDE][bgTypeId].begin(), battleMastersCache[HORDE][bgTypeId].end());

    std::list<CreatureDataPair const*> bmGuids;

    for (auto entry : entries)
    {
        for(auto creaturePair : WorldPosition().getCreaturesNear(0, entry))
        {
            bmGuids.push_back(creaturePair);
        }        
    }

    return bmGuids;
}

CreatureDataPair const* BgMasterValue::Calculate()
{
    CreatureDataPair const* bmPair = NearestBm(false);

    if (!bmPair)
        bmPair = NearestBm(true);
   
    return bmPair;
}

CreatureDataPair const* BgMasterValue::NearestBm(bool allowDead)
{
    WorldPosition botPos(bot);

    std::list<CreatureDataPair const*> bmPairs = GAI_VALUE2(std::list<CreatureDataPair const*>, "bg masters", qualifier);

    float rDist;
    CreatureDataPair const* rbmPair = nullptr;

    for (auto& bmPair : bmPairs)
    {
        ObjectGuid bmGuid(HIGHGUID_UNIT, bmPair->second.id, bmPair->first);

        if (!bmPair)
            continue;

        WorldPosition bmPos(bmPair);

        float dist = botPos.distance(bmPos); //This is the aproximate travel distance.

        // Skip battlemasters that are not on the same map
        if (bot->GetMapId() != bmPos.getMapId())
            continue;

        //Did we already find a closer unit that is not dead?
        if (rbmPair && rDist <= dist)
            continue;

        CreatureInfo const* bmTemplate = ObjectMgr::GetCreatureTemplate(bmPair->second.id);

        if (!bmTemplate)
            continue;

        FactionTemplateEntry const* bmFactionEntry = sFactionTemplateStore.LookupEntry(bmTemplate->Faction);

        //Is the unit hostile?
        if (ai->getReaction(bmFactionEntry) < REP_NEUTRAL)
            continue;

        AreaTableEntry const* area = bmPos.GetArea();

        if (!area)
            continue;

        //Is the area hostile?
        if (area->team == 4 && bot->GetTeam() == ALLIANCE)
            continue;
        if (area->team == 2 && bot->GetTeam() == HORDE)
            continue;

        if (!allowDead)
        {
            Unit* unit = ai->GetUnit(bmPair);

            if (!unit)
                continue;

            //Is the unit dead?
            if (unit->GetDeathState() == DEAD)
                continue;
        }

        rbmPair = bmPair;
        rDist = dist;
    }

    return rbmPair;
}


BattleGroundTypeId RpgBgTypeValue::Calculate()
{
    GuidPosition guidPosition = AI_VALUE(GuidPosition, "rpg target");

    // check Deserter debuff
    if (!bot->CanJoinToBattleground())
        return BATTLEGROUND_TYPE_NONE;

    // check if has free queue slots
    if (!bot->HasFreeBattleGroundQueueId())
        return BATTLEGROUND_TYPE_NONE;

    if(guidPosition)
        for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
        {
            BattleGroundQueueTypeId queueTypeId = (BattleGroundQueueTypeId)i;

            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);

            BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            if (!bg)
                continue;

            if (bot->GetLevel() < bg->GetMinLevel())
                continue;

            // check if already in queue
            if (bot->InBattleGroundQueueForBattleGroundQueueType(queueTypeId))
                continue;

            std::map<Team, std::map<BattleGroundTypeId, std::list<uint32>>> battleMastersCache = sRandomPlayerbotMgr.getBattleMastersCache();

            for (auto& entry : battleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;

            for (auto& entry : battleMastersCache[bot->GetTeam()][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;
        }

    return BATTLEGROUND_TYPE_NONE;
}

Unit* FlagCarrierValue::Calculate()
{
    Unit* carrier = nullptr;

    if (ai->GetBot()->InBattleGround())
    {
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
        {
            BattleGroundWS *bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

            if (!bg)
                return nullptr;

            if ((!sameTeam && bot->GetTeam() == HORDE || (sameTeam && bot->GetTeam() == ALLIANCE)) && !bg->GetFlagCarrierGuid(TEAM_INDEX_HORDE).IsEmpty())
                carrier = bg->GetBgMap()->GetPlayer(bg->GetFlagCarrierGuid(TEAM_INDEX_HORDE));

            if ((!sameTeam && bot->GetTeam() == ALLIANCE || (sameTeam && bot->GetTeam() == HORDE)) && !bg->GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE).IsEmpty())
                carrier = bg->GetBgMap()->GetPlayer(bg->GetFlagCarrierGuid(TEAM_INDEX_ALLIANCE));

            if (carrier)
            {
                if (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig.sightDistance))
                {
                    return carrier;
                }
                else
                    return nullptr;
            }
        }
#ifndef MANGOSBOT_ZERO
        if (ai->GetBot()->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_EY)
        {
            BattleGroundEY* bg = (BattleGroundEY*)ai->GetBot()->GetBattleGround();

            if (!bg)
                return nullptr;

            if (bg->GetFlagCarrierGuid().IsEmpty())
                return nullptr;

            Player* fc = bg->GetBgMap()->GetPlayer(bg->GetFlagCarrierGuid());
            if (!fc)
                return nullptr;

            if (!sameTeam && (fc->GetTeam() != bot->GetTeam()))
                carrier = fc;

            if (sameTeam && (fc->GetTeam() == bot->GetTeam()))
                carrier = fc;

            if (carrier)
            {
                if (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig.sightDistance))
                {
                    return carrier;
                }
                else
                    return nullptr;
            }
        }
#endif
    }
    return carrier;
}
