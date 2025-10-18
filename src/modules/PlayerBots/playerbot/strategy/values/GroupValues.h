#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class GroupMembersValue : public ObjectGuidListCalculatedValue
    {
    public:
        GroupMembersValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "group members",2) {}
        virtual std::list<ObjectGuid> Calculate();
    };

    class IsFollowingPartyValue : public BoolCalculatedValue
    {
    public:
        IsFollowingPartyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "following party") {}
        virtual bool Calculate();
    };

    class IsNearLeaderValue : public BoolCalculatedValue
    {
    public:
        IsNearLeaderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "near leader") {}
        virtual bool Calculate();
    };

    class GroupBoolCountValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        GroupBoolCountValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "group count"), Qualified() {}
        virtual uint32 Calculate();
    };
    
    class GroupBoolANDValue : public BoolCalculatedValue, public Qualified
    {
    public:
        GroupBoolANDValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group bool and"), Qualified() {}
        virtual bool Calculate();
    };

    class GroupBoolORValue : public BoolCalculatedValue, public Qualified
    {
    public:
        GroupBoolORValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group bool or"), Qualified() {}
        virtual bool Calculate();
    };

    class GroupReadyValue : public BoolCalculatedValue, public Qualified
    {
    public:
        GroupReadyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group ready", 2), Qualified() {}
        virtual bool Calculate();
    };
}
