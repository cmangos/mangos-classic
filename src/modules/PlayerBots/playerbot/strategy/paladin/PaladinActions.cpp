
#include "playerbot/playerbot.h"
#include "PaladinActions.h"

using namespace ai;

bool CastPaladinAuraAction::Execute(Event& event)
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
            haveAuras.push_back(aura);
    }

    if (haveAuras.empty())
    {
        return false;
    }

    for (auto aura : haveAuras)
    {
        if (!ai->HasAura(aura, bot))
        {
            uint32 spellDuration = sPlayerbotAIConfig.globalCoolDown;
            bool executed = ai->CastSpell(aura, bot, nullptr, false);
            if (executed)
            {
                SetDuration(1.0f);
            }

            return executed;
        }
    }

    return false;
}

Unit* CastBlessingAction::GetTarget()
{
    return bot;
}

bool CastBlessingAction::isPossible()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::string blessing = GetBlessingForTarget(target);
        if (!blessing.empty())
        {
            SetSpellName(blessing);
            return CastSpellAction::isPossible();
        }
    }

    return false;
}

std::string CastBlessingAction::GetBlessingForTarget(Unit* target)
{
    std::string chosenBlessing = "";
    if (target)
    {
        std::vector<std::string> possibleBlessings = GetPossibleBlessingsForTarget(target);
        for (const std::string& blessing : possibleBlessings)
        {
            const std::string greaterBlessing = "greater " + blessing;
            if ((greater || !ai->HasAura(blessing, target)) && !ai->HasAura(greaterBlessing, target))
            {
                if ((greater && ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true)) ||
                    (!greater && ai->CanCastSpell(blessing, target, 0, nullptr, true)))
                {
                    chosenBlessing = greater ? greaterBlessing : blessing;
                    break;
                }
            }
        }
    }

    return chosenBlessing;
}

std::vector<std::string> CastPveBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }

    return blessings;
}

std::vector<std::string> CastPvpBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }

    return blessings;
}

std::vector<std::string> CastRaidBlessingAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }

    return blessings;
}

Unit* CastBlessingOnPartyAction::GetTarget()
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
            haveBlessings.push_back("greater " + blessing);
        }
    }

    if (haveBlessings.empty())
    {
        return nullptr;
    }

    std::string blessList = "";
    for (auto blessing : haveBlessings)
    {
        blessList += blessing;
        if (blessing != haveBlessings[haveBlessings.size() - 1])
        {
            blessList += ",";
        }
    }

    return AI_VALUE2(Unit*, "party member without my aura", blessList);
}

bool CastBlessingOnPartyAction::isPossible()
{
    Unit* target = GetTarget();
    if (target)
    {
        std::string blessing = GetBlessingForTarget(target);
        if (!blessing.empty())
        {
            SetSpellName(blessing);
            return CastSpellAction::isPossible();
        }
    }

    return false;
}

std::string CastBlessingOnPartyAction::GetBlessingForTarget(Unit* target)
{
    std::string chosenBlessing = "";
    if (target)
    {
        std::vector<std::string> possibleBlessings = GetPossibleBlessingsForTarget(target);
        for (const std::string& blessing : possibleBlessings)
        {
            // Don't cast greater salvation on possible tank classes
            if (greater && blessing == "blessing of salvation" && target->IsPlayer())
            {
                const uint8 playerClass = ((Player*)target)->getClass();
#ifdef MANGOSBOT_TWO
                if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID || playerClass == CLASS_DEATH_KNIGHT)
#else
                if (playerClass == CLASS_PALADIN || playerClass == CLASS_WARRIOR || playerClass == CLASS_DRUID)
#endif
                {
                    break;
                }
            }

            const std::string greaterBlessing = "greater " + blessing;
            if ((greater || !ai->HasAura(blessing, target)) && !ai->HasAura(greaterBlessing, target))
            {
                if((greater && ai->CanCastSpell(greaterBlessing, target, 0, nullptr, true)) ||
                   (!greater && ai->CanCastSpell(blessing, target, 0, nullptr, true)))
                {
                    chosenBlessing = greater ? greaterBlessing : blessing;
                    break;
                }
            }
        }
    }

    return chosenBlessing;
}

std::vector<std::string> CastPveBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
        }
        else if (ai->IsRanged(player))
        {
            if (player->getClass() == CLASS_HUNTER)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of wisdom", "blessing of light", "blessing of sanctuary", "blessing of might" };
            }
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }
    else
    {
        // Blessings for pets
        blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
    }

    return blessings;
}

std::vector<std::string> CastPvpBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of light", "blessing of sanctuary", "blessing of might" };
        }
        else if (ai->IsRanged(player))
        {
            if (player->getClass() == CLASS_HUNTER)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of wisdom", "blessing of light", "blessing of sanctuary", "blessing of might" };
            }
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }
    else
    {
        // Blessings for pets
        blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
    }

    return blessings;
}

std::vector<std::string> CastRaidBlessingOnPartyAction::GetPossibleBlessingsForTarget(Unit* target) const
{
    std::vector<std::string> blessings;
    if (target && target->IsPlayer())
    {
        Player* player = (Player*)target;
        if (ai->IsTank(player))
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of might", "blessing of sanctuary", "blessing of light", "blessing of wisdom" };
            }
        }
        else if (ai->IsHeal(player))
        {
            blessings = { "blessing of wisdom", "blessing of kings", "blessing of sanctuary", "blessing of light", "blessing of might" };
        }
        else if (ai->IsRanged(player))
        {
            if (player->getClass() == CLASS_HUNTER)
            {
                blessings = { "blessing of wisdom", "blessing of kings", "blessing of might", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of kings", "blessing of wisdom", "blessing of light", "blessing of sanctuary", "blessing of might" };
            }
        }
        else
        {
            if (player->getClass() == CLASS_PALADIN)
            {
                blessings = { "blessing of wisdom", "blessing of might", "blessing of kings", "blessing of light", "blessing of sanctuary" };
            }
            else
            {
                blessings = { "blessing of might", "blessing of kings", "blessing of light", "blessing of wisdom", "blessing of sanctuary" };
            }
        }
    }
    else
    {
        // Blessings for pets
        blessings = { "blessing of might", "blessing of kings", "blessing of sanctuary", "blessing of light" };
    }

    return blessings;
}