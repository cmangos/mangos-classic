#pragma once
#include "playerbot/RandomPlayerbotMgr.h"
#include "GenericActions.h"

namespace ai
{
    class LeaveGroupAction : public ChatCommandAction
    {
    public:
        LeaveGroupAction(PlayerbotAI* ai, std::string name = "leave") : ChatCommandAction(ai, name) {}

        virtual bool Execute(Event& event) override
        {            
            Player* master = event.getOwner();

            return Leave(master);
        }

        virtual bool isUsefulWhenStunned() override { return true; }

        virtual bool Leave(Player* player);
    };

    class PartyCommandAction : public LeaveGroupAction 
    {
    public:
        PartyCommandAction(PlayerbotAI* ai) : LeaveGroupAction(ai, "party command") {}

        virtual bool Execute(Event& event) override
        {
            WorldPacket& p = event.getPacket();
            p.rpos(0);
            uint32 operation;
            std::string member;

            p >> operation >> member;

            if (operation != PARTY_OP_LEAVE)
                return false;

            Player* master = GetMaster();
            if (master && member == master->GetName())
                return Leave(bot);

            return false;
        }
    };

    class UninviteAction : public LeaveGroupAction 
    {
    public:
        UninviteAction(PlayerbotAI* ai) : LeaveGroupAction(ai, "uninvite") {}

        virtual bool Execute(Event& event) override
        {
            WorldPacket& p = event.getPacket();

            if (p.GetOpcode() == CMSG_GROUP_UNINVITE)
            {
                p.rpos(0);
                std::string membername;
                p >> membername;

                // player not found
                if (!normalizePlayerName(membername))
                {
                    return false;
                }

                if (bot->GetName() == membername)
                    return Leave(bot);
            }

            if (p.GetOpcode() == CMSG_GROUP_UNINVITE_GUID)
            {
                p.rpos(0);
                ObjectGuid guid;
                p >> guid;

                if (bot->GetObjectGuid() == guid)
                    return Leave(bot);
            }

            return false;
        }
    };

    class LeaveFarAwayAction : public LeaveGroupAction 
    {
    public:
        LeaveFarAwayAction(PlayerbotAI* ai) : LeaveGroupAction(ai, "leave far away") {}

        virtual bool Execute(Event& event) override
        {
            return Leave(ai->GetGroupMaster());
        }

        virtual bool isUseful();

        virtual bool isUsefulWhenStunned() override { return true; }
    };
}
