
#include "playerbot/playerbot.h"
#include "DeadValues.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

GuidPosition GraveyardValue::Calculate()
{
    WorldPosition refPosition = bot, botPos(bot);

    if (getQualifier() == "master")
    {
        if (ai->GetGroupMaster() && ai->IsSafe(ai->GetGroupMaster()) && ai->GetGroupMaster()->GetMapId() == bot->GetMapId())
        {
            refPosition = ai->GetGroupMaster();
        }
    }
    else if (getQualifier() == "travel")
    {
        auto travelTarget = AI_VALUE(TravelTarget*, "travel target");

        if (travelTarget && travelTarget->GetPosition() && travelTarget->GetPosition()->getMapId() == bot->GetMapId())
        {
            refPosition = *travelTarget->GetPosition();
        }
    }
    else if (getQualifier() == "another closest appropriate")
    {
        //just get ANOTHER nearest appropriate for level (neutral or same team zone)
        if (auto anotherAppropriate = GetAnotherAppropriateClosestGraveyard())
        {
            return GuidPosition(0, anotherAppropriate);
        }
    }

    WorldSafeLocsEntry const* ClosestGrave = bot->GetMap()->GetGraveyardManager().GetClosestGraveYard(
        refPosition.getX(),
        refPosition.getY(),
        refPosition.getZ(),
        refPosition.getMapId(),
        bot->GetTeam()
    );

    if (!ClosestGrave)
    {
        sLog.outBasic(
            "ERROR: Unable to find closest graveyard in GraveyardValue, will return GuidPosition() which is 0,0,0 - bot #%d %s:%d <%s>",
            bot->GetGUIDLow(),
            bot->GetTeam() == ALLIANCE ? "A" : "H",
            bot->GetLevel(),
            bot->GetName()
        );
        return GuidPosition();
    }

    return GuidPosition(0, ClosestGrave);
}

WorldSafeLocsEntry const* GraveyardValue::GetAnotherAppropriateClosestGraveyard() const
{
    // near
    float distNear = std::numeric_limits<float>::max();
    WorldSafeLocsEntry const* entryNear = nullptr;

    // far
    WorldSafeLocsEntry const* entryFar = nullptr;

    Corpse* corpse = bot->GetCorpse(); //
    uint32 botMapId = corpse->GetMapId();
    uint32 botZoneId = corpse->GetZoneId();

    for (auto mapValues : sWorld.GetGraveyardManager().GetGraveyardMap())
    {
        uint32 locId = mapValues.first;
        GraveYardData const& graveyardData = mapValues.second;

        //skip non-neutral or hostile graveyards
        if (graveyardData.team != bot->GetTeam() && graveyardData.team != TEAM_BOTH_ALLOWED)
            continue;

        WorldSafeLocsEntry const* graveyardCoreEntry = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(graveyardData.safeLocId);

        //skip different maps (no need for other continents)
        if (graveyardCoreEntry->map_id != botMapId)
            continue;

        uint32 graveyardZoneId = sTerrainMgr.GetZoneId(graveyardCoreEntry->map_id, graveyardCoreEntry->x, graveyardCoreEntry->y, graveyardCoreEntry->z);
        auto graveyardAreaEntry = GetAreaEntryByAreaID(graveyardZoneId);

        //skip same zone
        if (graveyardZoneId == botZoneId)
            continue;

        if (!graveyardAreaEntry)
            continue;

        //skip higher level zones
        if (bot->GetLevel() + 5 < (uint32)graveyardAreaEntry->area_level)
            continue;

        float dist = WorldPosition(corpse).sqDistance(graveyardCoreEntry);

        if (dist < distNear)
        {
            distNear = dist;
            entryNear = graveyardCoreEntry;
        }
    }

    if (entryNear)
        return entryNear;

    return entryFar;
}

GuidPosition BestGraveyardValue::Calculate()
{
    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        sLog.outBasic(
            "ERROR: Unable to find closest graveyard in BestGraveyardValue, will return GuidPosition() which is 0,0,0 - bot #%d %s:%d <%s>",
            bot->GetGUIDLow(),
            bot->GetTeam() == ALLIANCE ? "A" : "H",
            bot->GetLevel(),
            bot->GetName()
        );
        return GuidPosition();
    }

    uint32 deathCount = AI_VALUE(uint32, "death count");

    //attempt to revive at other same map graveyards which are not enemy territory
    if (!ai->HasActivePlayerMaster() && deathCount >= DEATH_COUNT_BEFORE_TRYING_ANOTHER_GRAVEYARD)
    {
        GuidPosition anotherGraveyard = AI_VALUE2(GuidPosition, "graveyard", "another closest appropriate");
        if (anotherGraveyard)
        {
            return anotherGraveyard;
        }
        sLog.outBasic(
            "ERROR: Unable to find another closest appropriate graveyard in BestGraveyardValue, resorting to self graveyard - bot #%d %s:%d <%s>",
            bot->GetGUIDLow(),
            bot->GetTeam() == ALLIANCE ? "A" : "H",
            bot->GetLevel(),
            bot->GetName()
        );
    }

    //Revive near master.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && ai->GetGroupMaster() && ai->GetGroupMaster() != bot)
    {
        GuidPosition masterGraveyard = AI_VALUE2(GuidPosition, "graveyard", "master");
        if (masterGraveyard)
        {
            return masterGraveyard;
        }
        sLog.outBasic(
            "ERROR: Unable to find master graveyard in BestGraveyardValue, resorting to self graveyard - bot #%d %s:%d <%s>",
            bot->GetGUIDLow(),
            bot->GetTeam() == ALLIANCE ? "A" : "H",
            bot->GetLevel(),
            bot->GetName()
        );
    }

    //Revive near travel target if it's far away from last death.
    if (AI_VALUE2(GuidPosition, "graveyard", "travel") && AI_VALUE2(GuidPosition, "graveyard", "travel").fDist(corpse) > sPlayerbotAIConfig.reactDistance)
    {
        GuidPosition travelGraveyard = AI_VALUE2(GuidPosition, "graveyard", "travel");
        if (travelGraveyard)
        {
            return travelGraveyard;
        }
        sLog.outBasic(
            "ERROR: Unable to find travel graveyard in BestGraveyardValue, resorting to self graveyard - bot #%d %s:%d <%s>",
            bot->GetGUIDLow(),
            bot->GetTeam() == ALLIANCE ? "A" : "H",
            bot->GetLevel(),
            bot->GetName()
        );
    }

    return AI_VALUE2(GuidPosition, "graveyard", "self");
}

bool ShouldSpiritHealerValue::Calculate()
{
    uint32 deathCount = AI_VALUE(uint32, "death count");
    uint8 durability = AI_VALUE(uint8, "durability");

    if (ai->HasActivePlayerMaster()) //Only use spirit healers with direct command with active master.
        return false;

    //Nothing to lose
    if (ai->HasAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS, bot) || durability < 10)
        return true;

    //Died too many times
    if (deathCount > DEATH_COUNT_BEFORE_REVIVING_AT_SPIRIT_HEALER)
        return true;

    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        //if no corpse (?) then definitely should revive at spirit healer
        return true;
    }

    uint32 deadTime = time(nullptr) - corpse->GetGhostTime();

    //Dead for a long time
    if (deadTime > 10 * MINUTE && deathCount > 1)
        return true;

    //Dead for a long time
    if (deadTime > 20 * MINUTE)
        return true;

    //If there are enemies near grave and corpse we go to corpse first.
    if (AI_VALUE2(bool, "manual bool", "enemies near graveyard"))
        return false;

    //Enemies near corpse so try grave first.
    if (AI_VALUE2(bool, "manual bool", "enemies near corpse"))
        return true;

    GuidPosition graveyard = AI_VALUE(GuidPosition, "best graveyard");

    float corpseDistance = WorldPosition(bot).fDist(corpse);
    float graveYardDistance = WorldPosition(bot).fDist(graveyard);
    bool corpseInSight = corpseDistance < sPlayerbotAIConfig.sightDistance;
    bool graveInSight = graveYardDistance < sPlayerbotAIConfig.sightDistance;
    bool enemiesNear = !AI_VALUE(std::list<ObjectGuid>, "possible targets").empty();

    if (enemiesNear)
    {
        if (graveInSight)
        {
            SET_AI_VALUE2(bool, "manual bool", "enemies near graveyard", true);
            return false;
        }
        if (corpseInSight)
        {
            SET_AI_VALUE2(bool, "manual bool", "enemies near corpse", true);
            return true;
        }
    }

    //If grave is near and no ress sickness go there.
    if (graveInSight && !corpseInSight && ai->HasCheat(BotCheatMask::repair))
        return true;

    //Stick to corpse.
    return false;
}