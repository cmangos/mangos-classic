
#include "playerbot/playerbot.h"
#include "PartyMemberWithoutAuraValue.h"

#include "playerbot/ServerFacade.h"
using namespace ai;

extern std::vector<std::string> split(const std::string &s, char delim);

class PlayerWithoutAuraPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutAuraPredicate(PlayerbotAI* ai, std::string aura) :
        PlayerbotAIAware(ai), FindPlayerPredicate(), auras(split(aura, ',')) {}

public:
    virtual bool Check(Unit* unit)
    {
        Pet* pet = dynamic_cast<Pet*>(unit);
        if (pet && (pet->getPetType() == MINI_PET || pet->getPetType() == SUMMON_PET))
            return false;

        if (!sServerFacade.IsAlive(unit)) return false;

        for (std::vector<std::string>::iterator i = auras.begin(); i != auras.end(); ++i)
        {
#ifdef MANGOSBOT_ZERO
            // Ignore mana buff spells for non mana users
            if (*i == "arcane intellect" || *i == "arcane brilliance" || *i == "divine spirit" || *i == "prayer of spirit")
            {
                if (unit->GetPowerType() != POWER_MANA)
                {
                    return false;
                }
            }
#endif

            if (ai->HasAura(*i, unit))
                return false;
        }

        return true;
    }

private:
    std::vector<std::string> auras;
};

Unit* FriendlyUnitWithoutAuraValue::Calculate()
{
    bool ignoreTank = false;
    std::string auras = qualifier;
    
    size_t paramPos = qualifier.find("-");
    if (paramPos != std::string::npos)
    {
        auras = qualifier.substr(0, paramPos);
        ignoreTank = std::stoi(qualifier.substr(paramPos + 1, paramPos + 2));
    }

    PlayerWithoutAuraPredicate predicate(ai, auras);
    return FindPartyMember(predicate, false, ignoreTank);
}

Unit* PartyMemberWithoutAuraValue::Calculate()
{
    bool ignoreTank = false;
    std::string auras = qualifier;

    size_t paramPos = qualifier.find("-");
    if (paramPos != std::string::npos)
    {
        auras = qualifier.substr(0, paramPos);
        ignoreTank = std::stoi(qualifier.substr(paramPos + 1, paramPos + 2));
    }

	PlayerWithoutAuraPredicate predicate(ai, auras);
    return FindPartyMember(predicate, true, ignoreTank);
}

class PlayerWithoutMyAuraPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutMyAuraPredicate(PlayerbotAI* ai, std::string aura) :
        PlayerbotAIAware(ai), FindPlayerPredicate(), auras(split(aura, ',')) {}

public:
    virtual bool Check(Unit* unit)
    {
        Pet* pet = dynamic_cast<Pet*>(unit);
        if (pet && (pet->getPetType() == MINI_PET || pet->getPetType() == SUMMON_PET))
            return false;

        if (!sServerFacade.IsAlive(unit)) return false;
        if (sServerFacade.GetDistance2d(unit, ai->GetBot()) > 30.0f) return false;

        for (std::vector<std::string>::iterator i = auras.begin(); i != auras.end(); ++i)
        {
            if (ai->HasMyAura(*i, unit))
                return false;
        }

        return true;
    }

private:
    std::vector<std::string> auras;
};

Unit* PartyMemberWithoutMyAuraValue::Calculate()
{
    bool ignoreTank = false;
    std::string auras = qualifier;

    size_t paramPos = qualifier.find("-");
    if (paramPos != std::string::npos)
    {
        auras = qualifier.substr(0, paramPos);
        ignoreTank = std::stoi(qualifier.substr(paramPos + 1, paramPos + 2));
    }

    PlayerWithoutMyAuraPredicate predicate(ai, auras);
    return FindPartyMember(predicate, false, ignoreTank);
}

class TankWithoutAuraPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    TankWithoutAuraPredicate(PlayerbotAI* ai, std::string aura) :
        PlayerbotAIAware(ai), FindPlayerPredicate(), auras(split(aura, ',')) {}

public:
    virtual bool Check(Unit* unit)
    {
        if (unit && unit->IsPlayer() && sServerFacade.IsAlive(unit))
        {
            if (ai->IsTank((Player*)unit))
            {
                bool missingAura = false;
                for (std::vector<std::string>::iterator i = auras.begin(); i != auras.end(); ++i)
                {
                    if (!ai->HasAura(*i, unit))
                    {
                        missingAura = true;
                        break;
                    }
                }

                return missingAura;
            }
        }

        return false;
    }

private:
    std::vector<std::string> auras;
};

Unit* PartyTankWithoutAuraValue::Calculate()
{
    TankWithoutAuraPredicate predicate(ai, qualifier);
    return FindPartyMember(predicate);
}
