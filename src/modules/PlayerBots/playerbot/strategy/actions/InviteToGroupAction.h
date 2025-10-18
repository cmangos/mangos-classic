#pragma once
#include "GenericActions.h"

namespace ai
{
    class InviteToGroupAction : public ChatCommandAction
    {
    public:
        InviteToGroupAction(PlayerbotAI* ai, std::string name = "invite") : ChatCommandAction(ai, name) {}

        virtual bool Execute(Event& event) override
        {
            Player* master = event.getOwner();
            return Invite(bot, master);
        }

        virtual bool Invite(Player* inviter, Player* player);
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class JoinGroupAction : public InviteToGroupAction
    {
    public:
        JoinGroupAction(PlayerbotAI* ai, std::string name = "join") : InviteToGroupAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class LfgAction : public InviteToGroupAction
    {
    public:
        LfgAction(PlayerbotAI* ai, std::string name = "lfg") : InviteToGroupAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class InviteNearbyToGroupAction : public InviteToGroupAction
    {
    public:
        InviteNearbyToGroupAction(PlayerbotAI* ai, std::string name = "invite nearby") : InviteToGroupAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful();
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    //Generic guid member finder
    class FindGuildMembers
    {
    public:
        FindGuildMembers() {};

        void operator()(Player* player) { data.push_back(player); };
        std::vector<Player*> const GetResult() { return data; };
    private:
        std::vector<Player*> data;
    };

    class InviteGuildToGroupAction : public InviteNearbyToGroupAction
    {
    public:
        InviteGuildToGroupAction(PlayerbotAI* ai, std::string name = "invite guild") : InviteNearbyToGroupAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() { return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful(); };

    private:
        std::vector<Player*> getGuildMembers();
    };
}
