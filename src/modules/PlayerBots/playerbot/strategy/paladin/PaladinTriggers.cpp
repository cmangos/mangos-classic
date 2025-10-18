
#include "playerbot/playerbot.h"
#include "PaladinTriggers.h"
#include "PaladinActions.h"

using namespace ai;

bool SealTrigger::IsActive()
{
	Unit* target = GetTarget();
	return !ai->HasAura("seal of justice", target) &&
        !ai->HasAura("seal of command", target) &&
        !ai->HasAura("seal of vengeance", target) &&
		!ai->HasAura("seal of righteousness", target) &&
		!ai->HasAura("seal of light", target) &&
        !ai->HasAura("seal of wisdom", target) &&
        AI_VALUE2(bool, "combat", "self target");
}

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::vector<std::string> altBlessings;
        std::vector<std::string> haveBlessings;
        altBlessings.push_back("blessing of might");
        altBlessings.push_back("blessing of wisdom");
        altBlessings.push_back("blessing of kings");
        altBlessings.push_back("blessing of sanctuary");
        altBlessings.push_back("blessing of salvation");
        altBlessings.push_back("blessing of light");

        for (auto blessing : altBlessings)
        {
            if (AI_VALUE2(uint32, "spell id", blessing))
            {
                haveBlessings.push_back(blessing);

                const std::string greaterBlessing = "greater " + blessing;
                if (AI_VALUE2(uint32, "spell id", greaterBlessing))
                {
                    haveBlessings.push_back(greaterBlessing);
                }
            }
        }

        if (haveBlessings.empty())
        {
            return false;
        }

        bool noBlessings = true;
        for (auto blessing : haveBlessings)
        {
            if (ai->HasMyAura(blessing, bot))
            {
                noBlessings = false;
            }
        }

        return noBlessings;
    }

    return false;
}

bool GreaterBlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::vector<std::string> altBlessings;
        std::vector<std::string> haveBlessings;
        altBlessings.push_back("blessing of might");
        altBlessings.push_back("blessing of wisdom");
        altBlessings.push_back("blessing of kings");
        altBlessings.push_back("blessing of sanctuary");
        altBlessings.push_back("blessing of salvation");
        altBlessings.push_back("blessing of light");

        for (auto blessing : altBlessings)
        {
            const std::string greaterBlessing = "greater " + blessing;
            if (AI_VALUE2(uint32, "spell id", greaterBlessing))
            {
                haveBlessings.push_back(greaterBlessing);
            }
        }

        if (haveBlessings.empty())
        {
            return false;
        }

        bool noBlessings = true;
        for (auto blessing : haveBlessings)
        {
            if (ai->HasMyAura(blessing, bot))
            {
                noBlessings = false;
            }
        }

        return noBlessings;
    }

    return false;
}

bool BlessingOnPartyTrigger::IsActive()
{
    std::vector<std::string> altBlessings;
    std::vector<std::string> haveBlessings;
    altBlessings.push_back("blessing of might");
    altBlessings.push_back("blessing of wisdom");
    altBlessings.push_back("blessing of kings");
    altBlessings.push_back("blessing of sanctuary");
    altBlessings.push_back("blessing of salvation");
    altBlessings.push_back("blessing of light");

    for (auto blessing : altBlessings)
    {
        if (AI_VALUE2(uint32, "spell id", blessing))
        {
            haveBlessings.push_back(blessing);

            const std::string greaterBlessing = "greater " + blessing;
            if (AI_VALUE2(uint32, "spell id", greaterBlessing))
            {
                haveBlessings.push_back(greaterBlessing);
            }
        }
    }

    if (haveBlessings.empty())
    {
        return false;
    }

    std::string blessings = "";
    for (auto blessing : haveBlessings)
    {
        blessings += blessing;
        if (blessing != haveBlessings[haveBlessings.size() - 1])
        {
            blessings += ",";
        }
    }

    // Doesn't have any of my blessings
    return AI_VALUE2(Unit*, "party member without my aura", blessings);
}

bool GreaterBlessingOnPartyTrigger::IsActive()
{
    std::vector<std::string> altBlessings;
    std::vector<std::string> haveBlessings;
    altBlessings.push_back("blessing of might");
    altBlessings.push_back("blessing of wisdom");
    altBlessings.push_back("blessing of kings");
    altBlessings.push_back("blessing of sanctuary");
    altBlessings.push_back("blessing of salvation");
    altBlessings.push_back("blessing of light");

    for (auto blessing : altBlessings)
    {
        const std::string greaterBlessing = "greater " + blessing;
        if (AI_VALUE2(uint32, "spell id", greaterBlessing))
        {
            haveBlessings.push_back(greaterBlessing);
        }
    }

    if (haveBlessings.empty())
    {
        return false;
    }

    std::string blessings = "";
    for (auto blessing : haveBlessings)
    {
        blessings += blessing;
        if (blessing != haveBlessings[haveBlessings.size() - 1])
        {
            blessings += ",";
        }
    }

    // Doesn't have any of my blessings
    Unit* target = AI_VALUE2(Unit*, "party member without my aura", blessings);
    return target && bot->IsInGroup(target);
}

bool NoPaladinAuraTrigger::IsActive()
{
    std::vector<std::string> altAuras;
    std::vector<std::string> haveAuras;
    altAuras.push_back("devotion aura");
    altAuras.push_back("retribution aura");
    altAuras.push_back("concentration aura");
    altAuras.push_back("sanctity aura");
    altAuras.push_back("shadow resistance aura");
    altAuras.push_back("fire resistance aura");
    altAuras.push_back("frost resistance aura");
    altAuras.push_back("crusader aura");

    for (auto aura : altAuras)
    {
        if (AI_VALUE2(uint32, "spell id", aura))
        {
            haveAuras.push_back(aura);
        }
    }

    if (haveAuras.empty())
    {
        return false;
    }

    bool hasAura = false;
    for (auto aura : haveAuras)
    {
        if (ai->HasMyAura(aura, bot))
        {
            hasAura = true;
            break;
        }
    }

    if (hasAura)
    {
        return false;
    }

    bool needAura = false;
    for (auto aura : haveAuras)
    {
        if (!ai->HasAura(aura, bot))
        {
            needAura = true;
            break;
        }
    }

    return needAura;
}

bool HammerOfJusticeOnEnemyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    uint8 targetHP = AI_VALUE2(uint8, "health", GetTargetName());
    uint8 selfHP = AI_VALUE2(uint8, "health", "self target");
    uint8 selfMP = AI_VALUE2(uint8, "mana", "self target");
    bool isMoving = AI_VALUE2(bool, "moving", GetTargetName());

    if (isMoving && target->IsPlayer())
        return true;

    if (targetHP < 10)
        return true;

    if (selfHP < sPlayerbotAIConfig.lowHealth && selfMP > 10)
        return true;

    return false;
}

bool ConsecrationTrigger::IsActive()
{
    if (SpellNoCooldownTrigger::IsActive())
    {
#ifdef MANGOSBOT_TWO
        return true;
#else
        return AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumMana;
#endif
    }

    return false;
}

bool ExorcismTrigger::IsActive()
{
    if (SpellNoCooldownTrigger::IsActive())
    {
#ifdef MANGOSBOT_TWO
        return ai->HasAura("the art of war", bot);
#else
        return AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumMana;
#endif
    }

    return false;
}
