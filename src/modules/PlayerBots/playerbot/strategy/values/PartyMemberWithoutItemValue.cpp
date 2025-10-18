
#include "playerbot/playerbot.h"
#include "PartyMemberWithoutItemValue.h"

#include "playerbot/ServerFacade.h"
using namespace ai;

class PlayerWithoutItemPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutItemPredicate(PlayerbotAI* ai, std::string item) :
        PlayerbotAIAware(ai), FindPlayerPredicate(), item(item) {}

public:
    virtual bool Check(Unit* unit)
    {
        Pet* pet = dynamic_cast<Pet*>(unit);
        if (pet && (pet->getPetType() == MINI_PET || pet->getPetType() == SUMMON_PET))
            return false;

        if (!sServerFacade.IsAlive(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

#ifdef MANGOS
        if (!(member->IsInSameGroupWith(ai->GetBot()) || member->IsInSameRaidWith(ai->GetBot())))
#endif
#ifdef CMANGOS
        if (!member->IsInGroup(ai->GetBot()))
#endif
            return false;

        PlayerbotAI *botAi = member->GetPlayerbotAI();
        if (!botAi)
            return false;

        return !botAi->GetAiObjectContext()->GetValue<uint32>("item count", item)->Get();
    }

private:
    std::string item;
};

Unit* PartyMemberWithoutItemValue::Calculate()
{
    FindPlayerPredicate *predicate = CreatePredicate();
    Unit *unit = FindPartyMember(*predicate);
    delete predicate;
    return unit;
}

FindPlayerPredicate* PartyMemberWithoutItemValue::CreatePredicate()
{
    return new PlayerWithoutItemPredicate(ai, qualifier);
}

class PlayerWithoutFoodPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutFoodPredicate(PlayerbotAI* ai) : PlayerWithoutItemPredicate(ai, "conjured food") {}

public:
    virtual bool Check(Unit* unit)
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        return member->getClass() != CLASS_MAGE;
    }

};

class PlayerWithoutWaterPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutWaterPredicate(PlayerbotAI* ai) : PlayerWithoutItemPredicate(ai, "conjured water") {}

public:
    virtual bool Check(Unit* unit)
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        uint8 cls = member->getClass();
        return cls == CLASS_DRUID ||
                cls == CLASS_HUNTER ||
                cls == CLASS_PALADIN ||
                cls == CLASS_PRIEST ||
                cls == CLASS_SHAMAN ||
                cls == CLASS_WARLOCK;
    }

};

FindPlayerPredicate* PartyMemberWithoutFoodValue::CreatePredicate()
{
    return new PlayerWithoutFoodPredicate(ai);
}

FindPlayerPredicate* PartyMemberWithoutWaterValue::CreatePredicate()
{
    return new PlayerWithoutWaterPredicate(ai);
}