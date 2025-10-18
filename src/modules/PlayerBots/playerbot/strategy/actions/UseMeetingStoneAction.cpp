
#include "playerbot/playerbot.h"
#include "UseMeetingStoneAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

#include "playerbot/strategy/values/PositionValue.h"
#include "Entities/Transports.h"

using namespace MaNGOS;

bool UseMeetingStoneAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid guid;
    p >> guid;

	if (requester->GetSelectionGuid() && requester->GetSelectionGuid() != bot->GetObjectGuid())
		return false;

	if (!requester->GetSelectionGuid() && requester->GetGroup() != bot->GetGroup())
		return false;

    if (requester->IsBeingTeleported())
        return false;

    if (sServerFacade.IsInCombat(bot))
    {
        ai->TellPlayerNoFacing(requester, "I am in combat");
        return false;
    }

    Map* map = requester->GetMap();
    if (!map)
        return false;

    GameObject *gameObject = map->GetGameObject(guid);
    if (!gameObject)
        return false;

	const GameObjectInfo* goInfo = gameObject->GetGOInfo();
	if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SUMMONING_RITUAL)
        return false;

    return Teleport(requester, requester, bot);
}

class AnyGameObjectInObjectRangeCheck
{
public:
    AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(GameObject* u)
    {
        if (u && i_obj->IsWithinDistInMap(u, i_range) && sServerFacade.isSpawned(u) && u->GetGOInfo())
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
};

bool SummonAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester || requester->IsBeingTeleported())
        return false;

    if (requester->GetSession()->GetSecurity() > SEC_PLAYER || sPlayerbotAIConfig.nonGmFreeSummon)
        return Teleport(requester, requester, bot);

    if(bot->GetMapId() == requester->GetMapId() && !WorldPosition(bot).canPathTo(requester, bot) && bot->GetDistance(requester) < sPlayerbotAIConfig.sightDistance) //We can't walk to requester so fine to short-range teleport.
        return Teleport(requester, requester, bot);

    if (bot->IsTaxiFlying())
        return false;

    if (SummonUsingGos(requester, requester, bot) || SummonUsingNpcs(requester, requester, bot))
    {
        ai->TellPlayerNoFacing(requester, BOT_TEXT("hello"));
        return true;
    }

    if (SummonUsingGos(requester, bot, requester) || SummonUsingNpcs(requester, bot, requester))
    {
        ai->TellPlayerNoFacing(requester, "Welcome!");
        return true;
    }

    return false;
}

bool SummonAction::SummonUsingGos(Player* requester, Player *summoner, Player *player)
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig.sightDistance);
    GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects((const WorldObject*)summoner, searcher, sPlayerbotAIConfig.sightDistance);

    for(std::list<GameObject*>::iterator tIter = targets.begin(); tIter != targets.end(); ++tIter)
    {
        GameObject* go = *tIter;
        if (go && sServerFacade.isSpawned(go) && go->GetGoType() == GAMEOBJECT_TYPE_MEETINGSTONE)
            return Teleport(requester, summoner, player);
    }

    ai->TellPlayerNoFacing(requester, summoner == bot ? "There is no meeting stone nearby" : "There is no meeting stone near you");
    return false;
}

bool SummonAction::SummonUsingNpcs(Player* requester, Player *summoner, Player *player)
{
    if (!sPlayerbotAIConfig.summonAtInnkeepersEnabled)
        return false;

    std::list<Unit*> targets;
    AnyUnitInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig.sightDistance);
    UnitListSearcher<AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(summoner, searcher, sPlayerbotAIConfig.sightDistance);
    for(std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end(); ++tIter)
    {
        Unit* unit = *tIter;
        if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER))
        {
            if (!player->HasItemCount(6948, 1, false))
            {
                ai->TellPlayerNoFacing(requester, player == bot ? "I have no hearthstone" : "You have no hearthstone");
                return false;
            }

            if (!sServerFacade.IsSpellReady(player, 8690))
            {
                ai->TellPlayerNoFacing(requester, player == bot ? "My hearthstone is not ready" : "Your hearthstone is not ready");
                return false;
            }

            // Trigger cooldown
            SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(8690);
            if (!spellInfo)
                return false;
            Spell spell(player, spellInfo,
#ifdef MANGOS
                    0
#endif
#ifdef CMANGOS
                    TRIGGERED_OLD_TRIGGERED
#endif
                    );
            spell.SendSpellCooldown();

            return Teleport(requester, summoner, player);
        }
    }

    ai->TellPlayerNoFacing(requester, summoner == bot ? "There are no innkeepers nearby" : "There are no innkeepers near you");
    return false;
}

bool SummonAction::Teleport(Player* requester, Player *summoner, Player *player)
{
    if (!summoner->IsBeingTeleported() && !player->IsBeingTeleported() && summoner != player)
    {
        float followAngle = GetFollowAngle();
        for (double angle = followAngle - M_PI; angle <= followAngle + M_PI; angle += M_PI / 4)
        {
            uint32 mapId = summoner->GetMapId();
            float x = summoner->GetPositionX() + cos(angle) * ai->GetRange("follow");
            float y = summoner->GetPositionY() + sin(angle) * ai->GetRange("follow");
            float z = summoner->GetPositionZ();
            summoner->UpdateGroundPositionZ(x, y, z);

            if (!summoner->IsWithinLOS(x, y, z + player->GetCollisionHeight(), true))
            {
                x = summoner->GetPositionX();
                y = summoner->GetPositionY();
                z = summoner->GetPositionZ();
            }

            if (summoner->IsWithinLOS(x, y, z + player->GetCollisionHeight(), true))
            {
                if (sServerFacade.UnitIsDead(player) && sServerFacade.IsAlive(summoner))
                {
                    if (!ai->IsSafe(player) || !ai->IsSafe(summoner))
                        return false;

                    player->ResurrectPlayer(1.0f, false);
                    player->SpawnCorpseBones();
                    ai->TellPlayerNoFacing(requester, "I live, again!");
                }

                if (player->IsTaxiFlying())
                {
                    player->TaxiFlightInterrupt();
                    player->GetMotionMaster()->MovementExpired();
                }

                player->GetMotionMaster()->Clear();
                player->TeleportTo(mapId, x, y, z, 0);
                if(player->isRealPlayer())
                    player->SendHeartBeat();

                if (summoner->GetTransport())
                    summoner->GetTransport()->AddPassenger(player, false);
                    
                if(ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT))
                    SET_AI_VALUE2(PositionEntry, "pos", "stay", PositionEntry(x, y, z, mapId));
                if (ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
                    SET_AI_VALUE2(PositionEntry, "pos", "guard", PositionEntry(x, y, z, mapId));

                return true;
            }
        }
    }

    if(summoner != player)
        ai->TellPlayerNoFacing(requester, "Not enough place to summon");
    return false;
}

bool AcceptSummonAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid summonerGuid;
    p >> summonerGuid;

    WorldPacket response(CMSG_SUMMON_RESPONSE);
    response << summonerGuid;
#if defined(MANGOSBOT_ONE) || defined(MANGOSBOT_TWO)
    response << uint8(1);
#endif
    bot->GetSession()->HandleSummonResponseOpcode(response);
    
    return true;
}
