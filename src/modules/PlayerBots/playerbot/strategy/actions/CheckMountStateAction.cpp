
#include "playerbot/playerbot.h"
#include "CheckMountStateAction.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/MountValues.h"
#include "BattleGround/BattleGroundWS.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

uint64 extractGuid(WorldPacket& packet);

bool CheckMountStateAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Player* groupMaster = ai->GetGroupMaster();

    bool hasAttackers = AI_VALUE(bool, "has attackers");
    bool hasEnemy = AI_VALUE(bool, "has enemy player targets") || AI_VALUE(Unit*, "dps target");

    bool canFly = CanFly();

    bool farFromMaster = false;

    if(groupMaster)
        farFromMaster = bot->GetMapId() != groupMaster->GetMapId() || bot->GetDistance(groupMaster) > sPlayerbotAIConfig.sightDistance;

    bool canAttackTarget = false;
    bool shouldChaseTarget = false;
    bool farFromTarget = false;

    if (hasEnemy)
    {
        float distToTarget = AI_VALUE(Unit*, "current target") ? AI_VALUE2(float, "distance", "current target") : 0;
        canAttackTarget = sServerFacade.IsDistanceLessThan(distToTarget, GetAttackDistance());
        shouldChaseTarget = sServerFacade.IsDistanceGreaterThan(distToTarget, 45.0f) && AI_VALUE2(bool, "moving", "current target");
        farFromTarget = sServerFacade.IsDistanceGreaterThan(distToTarget, 40.0f);
    }

    bool IsMounted = AI_VALUE2(uint32, "current mount speed", "self target");
    bool IsLeaderMounted = groupMaster && AI_VALUE2(uint32, "current mount speed", "master target");

    //Mount up in battle grounds
    if (bot->InBattleGround())
    {
        if (WorldPosition(bot).currentHeight() < -5.0f)
            return UnMount();

        if (!canAttackTarget)
        {
            if (!hasAttackers || (farFromTarget && !bot->IsInCombat()))
            {
                if (CanMountInBg())
                {
                    if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                        ai->TellPlayerNoFacing(requester, "Mount in bg. No attackers or far from target and not in combat.");

                    return Mount(requester);
                }
            }
        }
    }

    //Unmounted when able to attack target
    if (canAttackTarget)
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
            ai->TellPlayerNoFacing(requester, "Unmount. Able to attack target.");
        return UnMount();
    }

    //No more logic needed in bg.
    if (bot->InBattleGround())
        return false;

    //Chase to current target.
    if (!canAttackTarget && (farFromTarget || shouldChaseTarget))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
            ai->TellPlayerNoFacing(requester, "Mount. Unable to attack target and target is far or chaseable.");
        
        return Mount(requester);
    }

    //Following master and close to master that is unmounted.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && groupMaster && groupMaster != bot && !farFromMaster && !IsLeaderMounted)
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
            ai->TellPlayerNoFacing(requester, "Unmount. Near umounted group master.");

        return UnMount();
    }

    //Near guard position
    if (ai->HasStrategy("guard", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry guardPosition = posMap["guard"];

        float distance = AI_VALUE2(float, "distance", "position_guard");

        if (guardPosition.isSet() && distance < ai->GetRange("follow"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
                ai->TellPlayerNoFacing(requester, "Unmount. Near umounted guard position.");

            return UnMount();
        }
    }

    //Near stay position
    if (ai->HasStrategy("stay", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry stayPosition = posMap["stay"];

        float distance = AI_VALUE2(float, "distance", "position_stay");

        if (stayPosition.isSet() && distance < ai->GetRange("follow"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
                ai->TellPlayerNoFacing(requester, "Unmount. Near stay location.");

            return UnMount();
        }
    }

    //Doing stuff nearby.
    if (AI_VALUE(bool, "travel target working"))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
            ai->TellPlayerNoFacing(requester, "Unmount. Near travel target.");

        return UnMount();
    }

    //Rping nearby.
    if (AI_VALUE(GuidPosition, "rpg target") && sServerFacade.IsDistanceLessThan(AI_VALUE2(float, "distance", "rpg target"), sPlayerbotAIConfig.farDistance))
    {
        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && IsMounted)
            ai->TellPlayerNoFacing(requester, "Unmount. Near rpg target.");

        return UnMount();
    }

    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && groupMaster && groupMaster != bot)
    {
        //Mounting with master.
        if (IsLeaderMounted && !hasAttackers)
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                ai->TellPlayerNoFacing(requester, "Mount. Group master mounted and no attackers.");

            return Mount(requester);
        }

        //Mounting to move to master.
        if (farFromMaster && !bot->IsInCombat())
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                ai->TellPlayerNoFacing(requester, "Mount. Far from group master and not in combat.");

            return Mount(requester);
        }
    }

    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !hasEnemy)
    {
        //Mounting to travel.
        if (AI_VALUE(bool, "travel target traveling") && AI_VALUE(bool, "can move around"))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                ai->TellPlayerNoFacing(requester, "Mount. Traveling some place.");

            return Mount(requester, true);
        }
        else if (!hasAttackers)
        {
            //Mounting to move to rpg target.
            if (AI_VALUE(GuidPosition, "rpg target") && sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "rpg target"), sPlayerbotAIConfig.sightDistance))
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                    ai->TellPlayerNoFacing(requester, "Mount. Rpg target far away.");

                return Mount(requester, true);
            }

            //Mounting in safe place.
            if (!ai->HasStrategy("guard", ai->GetState()) && !ai->HasStrategy("stay", ai->GetState()) && !AI_VALUE(std::list<ObjectGuid>, "possible rpg targets").empty() && urand(0, 100) > 50)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                    ai->TellPlayerNoFacing(requester, "Mount. Near rpg targets.");

                return Mount(requester, true);
            }
        }

        //Far from guard position
        if (ai->HasStrategy("guard", ai->GetState()))
        {
            PositionMap& posMap = AI_VALUE(PositionMap&, "position");
            PositionEntry guardPosition = posMap["guard"];

            float distance = AI_VALUE2(float, "distance", "position_guard");

            if (guardPosition.isSet() && distance > 40.0f)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                    ai->TellPlayerNoFacing(requester, "Mount. Move to guard.");

                return Mount(requester);
            }
        }

        //Far from stay position
        if (ai->HasStrategy("stay", ai->GetState()))
        {
            PositionMap& posMap = AI_VALUE(PositionMap&, "position");
            PositionEntry guardPosition = posMap["stay"];

            float distance = AI_VALUE2(float, "distance", "position_stay");

            if (guardPosition.isSet() && distance > 40.0f)
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT) && !IsMounted)
                    ai->TellPlayerNoFacing(requester, "Mount. Move to stay.");

                return Mount(requester);
            }
        }
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    // do not use on vehicle
    if (ai->IsInVehicle())
        return false;

    if (bot->IsDead())
        return false;

    if (!bot->IsInWorld())
        return false;

    bool isOutdoor = bot->GetMap()->GetTerrain()->IsOutdoors(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (!isOutdoor)
        return false;

    if (bot->IsTaxiFlying())
        return false;

    if (bot->getClass() == CLASS_ROGUE && bot->InBattleGround() && (ai->HasAura("stealth", bot) || ai->HasAura("sprint", bot)))
        return false;

    if (bot->getClass() == CLASS_DRUID && bot->InBattleGround() && (ai->HasAura("prowl", bot) || ai->HasAura("dash", bot)))
        return false;

#ifndef MANGOSBOT_ZERO
    if (bot->InArena())
        return false;
#endif

    if (!bot->GetPlayerbotAI()->HasStrategy("mount", BotState::BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    if (!bot->IsMounted() && bot->IsInWater())
        return false;

    bool firstmount = bot->GetLevel() >=
#ifdef MANGOSBOT_ZERO
        40
#else
#ifdef MANGOSBOT_ONE
        30
#else
        20
#endif
#endif
        ;
    if (!firstmount)
        return false;

    // Do not use with BG Flags
    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
    {
        return false;
    }

    // Only mount if BG starts in less than 30 sec
    if (bot->InBattleGround())
    {
        BattleGround* bg = bot->GetBattleGround();
        if (bg && bg->GetStatus() == STATUS_WAIT_JOIN)
        {
            if (bg->GetStartDelayTime() > BG_START_DELAY_30S)
                return false;
        }
    }

    if (!bot->GetMap()->IsMountAllowed() && bot->GetMapId() != 531)
        return false;

    if (AI_VALUE(std::vector<MountValue>, "mount list").empty())
        return false;

    return true;
}

bool CheckMountStateAction::CanFly() const
{
    if (bot->GetMapId() != 530 && bot->GetMapId() != 571)
        return false;

#ifdef MANGOSBOT_ONE
    uint32 zone, area;
    bot->GetZoneAndAreaId(zone, area);
    uint32 v_map = GetVirtualMapForMapAndZone(bot->GetMapId(), zone);
    MapEntry const* mapEntry = sMapStore.LookupEntry(v_map);
    if (!mapEntry || mapEntry->addon < 1 || !mapEntry->IsContinent())
        return false;
#endif
#ifdef MANGOSBOT_TWO
    uint32 zone, area;
    bot->GetZoneAndAreaId(zone, area);
    if (!bot->CanStartFlyInArea(bot->GetMapId(), zone, area, false))
        return false;
#endif

    for (auto& mount : AI_VALUE(std::vector<MountValue>, "mount list"))
        if (mount.GetSpeed(true))
            return true;

    return false;
}

bool CheckMountStateAction::CanMountInBg() const
{
    //Do not mount with or near flag.
    if (bot->GetBattleGroundTypeId() == BattleGroundTypeId::BATTLEGROUND_WS)
    {
        BattleGroundWS* bg = (BattleGroundWS*)ai->GetBot()->GetBattleGround();

        if (bot->HasAura(23333) || bot->HasAura(23335))
        {
            return false;
        }
#ifdef MANGOSBOT_ZERO
        //check near A Flag
        uint32 lowguid = 90000;
        uint32 id = 179830;
        if (!bg)
            return false;
        GameObject* AllianceflagStand = bg->GetBgMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, id, lowguid));

        if (bot->IsWithinDistInMap(AllianceflagStand, 3.0f))
            return false;

        //check near H Flag
        lowguid = 90001;
        id = 179831;
        GameObject* HordeflagStand = bg->GetBgMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, id, lowguid));

        if (bot->IsWithinDistInMap(HordeflagStand, 3.0f))
            return false;
#endif
    }
    return true;
}

float CheckMountStateAction::GetAttackDistance() const
{
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
        return 10.0f;
    case CLASS_ROGUE:
        return 50.0f;
    }
    /*if (enemy)
    attack_distance /= 2;*/

    if (ai->IsHeal(bot) || ai->IsRanged(bot))
        return 40.0f;

    return 35.0f;
}

bool CheckMountStateAction::Mount(Player* requester, bool limitSpeedToGroup)
{
    bool canFly = CanFly();   

    uint32 currentSpeed = AI_VALUE2(uint32, "current mount speed", "self target");

    uint32 maxSpeed = 9999;

    if (limitSpeedToGroup && bot->GetGroup() && ai->IsGroupLeader())
    {
        for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (member == bot)
                continue;

            if (!member->GetPlayerbotAI())
                continue;

            if (!member->IsAlive())
                continue;

            if (!member->GetPlayerbotAI()->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                continue;

            if (WorldPosition(bot).distance(member) > sPlayerbotAIConfig.reactDistance * 5)
                continue;

            Player* player = member;

            maxSpeed = std::min(maxSpeed, PAI_VALUE2(uint32, "max mount speed", canFly ? "fly" : ""));
        }
    }

    std::vector<MountValue> mountList = AI_VALUE(std::vector<MountValue>, "mount list");

    std::shuffle(mountList.begin(), mountList.end(), *GetRandomGenerator());
    std::sort(mountList.begin(), mountList.end(), [canFly](MountValue i, MountValue j) {return i.GetSpeed(canFly) > j.GetSpeed(canFly); });

    for (auto& mount : mountList)
    {
        if (mount.GetSpeed(canFly) > maxSpeed)
            continue;

        if (currentSpeed > maxSpeed)
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "Mounted, unmount to mount slower next time.");
            return UnMount();
        }

        if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(requester, "Try to mount with " + chat->formatSpell(mount.GetSpellId()));

        if (currentSpeed >= mount.GetSpeed(canFly))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "Speed not faster than current.");

            return false;
        }

        if (currentSpeed) //Already mounted
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "Mounted, unmount to mount faster next time.");
            return UnMount();
        }

        ai->RemoveShapeshift();

        bool wasMoving = sServerFacade.isMoving(bot);

        if (wasMoving)
        {
            ai->StopMoving();
        }

        bool didMount = false;

        if (!mount.IsValidLocation(bot))
        {
            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "Bot can not use this mount here.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            continue;
        }

        if (mount.IsItem())
        {
            if (!bot->GetItemByEntry(mount.GetItemProto()->ItemId))
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayerNoFacing(requester, "Bot does not have this mount.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
                continue;
            }

            if (UseItem(requester, mount.GetItemProto()->ItemId))
            {
                SetDuration(3000U); // 3s
                didMount = true;
            }
            else
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayerNoFacing(requester, "Mounting failed.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            }
        }
        else
        {
            if (!ai->HasSpell(mount.GetSpellId()))
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayerNoFacing(requester, "Bot does not have this mount spell.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
                continue;
            }
            uint32 castDuration;
            if (ai->CastSpell(mount.GetSpellId(), bot, nullptr, true, &castDuration))
            {
                sPlayerbotAIConfig.logEvent(ai, "CheckMountStateAction", sServerFacade.LookupSpellInfo(mount.GetSpellId())->SpellName[0], std::to_string(mount.GetSpeed(canFly)));
                SetDuration(castDuration);
                didMount = true;
            }
            else
            {
                if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayerNoFacing(requester, "Mounting spell failed.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            }
        }

        if (didMount)
        {
            if (wasMoving)
            {
                ai->HandleCommand(CHAT_MSG_WHISPER, "queue do check mount state", *bot);
                ai->TellDebug(requester, "was moving trying to mount again.", "debug mount");
            }

            if (ai->HasStrategy("debug mount", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "Mounting.");

            return didMount;
        }
    }

    return false;
}

bool CheckMountStateAction::UnMount() const
{
    if (!AI_VALUE2(uint32, "current mount speed", "self target"))
        return false;

    if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
    {
        return false;
    }

    if (bot->IsMounted())
    {
        ai->Unmount();
    }

    ai->RemoveShapeshift();

    return true;
}