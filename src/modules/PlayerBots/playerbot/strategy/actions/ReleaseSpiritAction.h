#pragma once

#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "ReviveFromCorpseAction.h"
#include "playerbot/TravelMgr.h"

namespace ai
{
    class ReleaseSpiritAction : public ChatCommandAction
    {
    public:
        ReleaseSpiritAction(PlayerbotAI* ai, std::string name = "release") : ChatCommandAction(ai, name) {}

    public:
        virtual bool Execute(Event& event) override
        {
            Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
            if (sServerFacade.IsAlive(bot))
                return false;

            if (bot->GetCorpse() && bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
            {
                ai->TellPlayerNoFacing(requester, "I am already a spirit", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                return false;
            }

            WorldPacket& p = event.getPacket();
            if (!p.empty() && p.GetOpcode() == CMSG_REPOP_REQUEST)
                ai->TellPlayerNoFacing(requester, "Releasing...", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            else
                ai->TellPlayerNoFacing(requester, "Meet me at the graveyard", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            sLog.outDetail("Bot #%d %s:%d <%s> released", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());

            WorldPacket packet(CMSG_REPOP_REQUEST);
            packet << uint8(0);
            bot->GetSession()->HandleRepopRequestOpcode(packet);

            // add waiting for ress aura
            if (bot->InBattleGround() && !ai->HasAura(2584, bot))
            {
                // cast Waiting for Resurrect
                bot->CastSpell(bot, 2584, TRIGGERED_OLD_TRIGGERED);
            }

            return true;
        }
    };

    class AutoReleaseSpiritAction : public ReleaseSpiritAction 
    {
    public:
        AutoReleaseSpiritAction(PlayerbotAI* ai, std::string name = "auto release") : ReleaseSpiritAction(ai, name) {}

        virtual bool Execute(Event& event) override
        {
            sLog.outDetail("Bot #%d %s:%d <%s> auto released", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());

            WorldPacket packet(CMSG_REPOP_REQUEST);
            packet << uint8(0);
            bot->GetSession()->HandleRepopRequestOpcode(packet);

            // add waiting for ress aura
            if (bot->InBattleGround() && !ai->HasAura(2584, bot))
            {
                // cast Waiting for Resurrect
                bot->CastSpell(bot, 2584, TRIGGERED_OLD_TRIGGERED);
            }

            sPlayerbotAIConfig.logEvent(ai, "AutoReleaseSpiritAction");

            return true;
        }

        virtual bool isUseful()
        {
            if (!sServerFacade.UnitIsDead(bot))
                return false;

            if (bot->GetCorpse())
                return false;

#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif

            if (bot->InBattleGround())
                return !bot->GetCorpse() || !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST);

            if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                return false;

            if (!bot->GetGroup())
                return true;

            if (!ai->GetGroupMaster())
                return true;

            if (ai->GetGroupMaster() == bot)
                return true;

            if (!ai->HasActivePlayerMaster())
                return true;

            if (ai->HasActivePlayerMaster() && ai->GetGroupMaster()->GetMapId() == bot->GetMapId() && (bot->GetMap()->IsRaid() || bot->GetMap()->IsDungeon()))
                return false;

            if(sServerFacade.UnitIsDead(ai->GetGroupMaster()))
                return true;

            if (sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig.sightDistance))
                return true;

            return false;
        }
    };

    class RepopAction : public SpiritHealerAction 
    {
    public:
        RepopAction(PlayerbotAI* ai, std::string name = "repop") : SpiritHealerAction(ai, name) {}

    public:
        virtual bool Execute(Event& event)
        {
            Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

            sLog.outBasic("Repop bot #%d %s:%d <%s>", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());

            SET_AI_VALUE(uint32, "death count", 0);

            if (bot->IsDead())
            {
                bot->ResurrectPlayer(1.0f, false);
                bot->SpawnCorpseBones();
                bot->SaveToDB();
            }

            
            if (!ai->HasRealPlayerMaster())
            {
                if (Group* group = bot->GetGroup())
                {
                    sLog.outBasic("Repop: Removing bot #%d %s:%d <%s> from group", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
                    group->RemoveMember(bot->GetObjectGuid(), 0);
                }
            }

            RESET_AI_VALUE(Unit*, "old target");
            RESET_AI_VALUE(Unit*, "current target");
            RESET_AI_VALUE(Unit*, "pull target");
            RESET_AI_VALUE(bool, "combat::self target");
            RESET_AI_VALUE(WorldPosition, "current position");

            bot->SetSelectionGuid(ObjectGuid());
            ai->TellPlayer(requester, BOT_TEXT("hello"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

            TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
            sTravelMgr.SetNullTravelTarget(travelTarget);
            travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_EXPIRED);
            travelTarget->SetExpireIn(1000);

            PlayerInfo const* defaultPlayerInfo = sObjectMgr.GetPlayerInfo(bot->getRace(), bot->getClass());
            if (defaultPlayerInfo)
            {
                sLog.outBasic("Repop: Teleporting bot #%d %s:%d <%s> to spawn", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
                //teleport bot to spawn
                bot->TeleportTo(defaultPlayerInfo->mapId, defaultPlayerInfo->positionX, defaultPlayerInfo->positionY, defaultPlayerInfo->positionZ, defaultPlayerInfo->orientation);
                if (bot->isRealPlayer())
                    bot->SendHeartBeat();
            }
            else
            {
                sLog.outBasic("Repop: Teleporting bot #%d %s:%d <%s> to homebind", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
                //teleport bot to homebind
                bot->TeleportToHomebind();
                bot->SendHeartBeat();
            }

            sPlayerbotAIConfig.logEvent(ai, "RepopAction");

            return true;
        }

        virtual bool isUseful()
        {
            if (bot->InBattleGround())
                return false;

            if (ai->HasActivePlayerMaster())
                return false;

            return true;
        }
    };

    class SelfResurrectAction : public ChatCommandAction
    {
    public:
        SelfResurrectAction(PlayerbotAI* ai, std::string name = "self resurrect") : ChatCommandAction(ai, name) {}

    public:
        bool Execute(Event& event) override
        {
            WorldPacket packet(CMSG_SELF_RES);
            bot->GetSession()->HandleSelfResOpcode(packet);
            return true;
        }

        bool isPossible() override
        {
            return ai->IsStateActive(BotState::BOT_STATE_DEAD) && bot->GetUInt32Value(PLAYER_SELF_RES_SPELL);
        }
    };
}
