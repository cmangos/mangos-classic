
#include "playerbot/playerbot.h"
#include "PartyMemberToDispel.h"

#include "playerbot/ServerFacade.h"
using namespace ai;

class PartyMemberToDispelPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PartyMemberToDispelPredicate(PlayerbotAI* ai, uint32 dispelType) :
        PlayerbotAIAware(ai), FindPlayerPredicate(), dispelType(dispelType) {}

public:
    virtual bool Check(Unit* unit)
    {
        Pet* pet = dynamic_cast<Pet*>(unit);
        if (pet && (pet->getPetType() == MINI_PET || pet->getPetType() == SUMMON_PET))
            return false;

        return sServerFacade.IsAlive(unit) && sServerFacade.GetDistance2d(ai->GetBot(), unit) <= ai->GetRange("spell") && ai->HasAuraToDispel(unit, dispelType);
    }

private:
    uint32 dispelType;
};

Unit* PartyMemberToDispel::Calculate()
{
    uint32 dispelType = atoi(qualifier.c_str());

    PartyMemberToDispelPredicate predicate(ai, dispelType);
    return FindPartyMember(predicate);
}
