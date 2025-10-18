#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class FindPlayerPredicate
    {
    public:
        virtual ~FindPlayerPredicate() {}
        virtual bool Check(Unit*) = 0;
    };

    class SpellEntryPredicate
    {
    public:
        virtual bool Check(SpellEntry const*) = 0;
    };

    class PartyMemberValue : public UnitCalculatedValue
	{
	public:
        PartyMemberValue(PlayerbotAI* ai, std::string name = "party member") : UnitCalculatedValue(ai, name) {}

    public:
        bool IsTargetOfSpellCast(Player* target, SpellEntryPredicate &predicate);

    protected:
        Unit* FindPartyMember(FindPlayerPredicate &predicate, bool ignoreOutOfGroup = false, bool ignoreTanks = false);
        Unit* FindPartyMember(std::list<Player*>* party, FindPlayerPredicate &predicate, bool ignoreTanks);
        bool Check(Unit* player);
	};
}
