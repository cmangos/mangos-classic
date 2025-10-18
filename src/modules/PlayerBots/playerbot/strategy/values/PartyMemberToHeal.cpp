
#include "playerbot/playerbot.h"
#include "PartyMemberToHeal.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

class IsTargetOfHealingSpell : public SpellEntryPredicate
{
public:
    virtual bool Check(SpellEntry const* spell) 
    {
        return PlayerbotAI::IsHealSpell(spell);
    }
};

uint32 getIncomingdamage(Unit const* pTarget)
{
    uint32 damage = 0;
    for (auto const& pAttacker : pTarget->getAttackers())
        if (pAttacker->CanReachWithMeleeAttack(pTarget))
            damage += uint32((pAttacker->GetFloatValue(UNIT_FIELD_MINDAMAGE) + pAttacker->GetFloatValue(UNIT_FIELD_MAXDAMAGE)) / 2);

    return damage;
}

bool compareByHealth(const Unit *u1, const Unit *u2)
{
    return u1->GetHealthPercent() < u2->GetHealthPercent();
}

bool compareByMissingHealth(const Unit* u1, const Unit* u2, bool incomingDamage = false)
{
    uint32 hp1 = u1->GetHealth() - (incomingDamage ? getIncomingdamage(u1) : 0);
    uint32 hpmax1 = u1->GetMaxHealth();
    uint32 hp2 = u2->GetHealth() - (incomingDamage ? getIncomingdamage(u2) : 0);
    uint32 hpmax2 = u2->GetMaxHealth();
    return (hpmax1 - hp1) > (hpmax2 - hp2);
}

Unit* PartyMemberToHeal::Calculate()
{
    std::vector<Unit*> needHeals;
    std::vector<Unit*> tankTargets;
    if (bot->GetSelectionGuid())
    {
        Unit* target = ai->GetUnit(bot->GetSelectionGuid());
        if (target &&
            target->GetObjectGuid() != bot->GetObjectGuid() && 
            sServerFacade.IsFriendlyTo(bot, target) &&
            target->GetHealthPercent() < 100 && 
            Check(target))
        {
            needHeals.push_back(target);
        }
    }

    if (GuidPosition rpgTarget = AI_VALUE(GuidPosition, "rpg target"))
    {
        Unit* target = rpgTarget.GetCreature(bot->GetInstanceId());
        if (target && sServerFacade.IsFriendlyTo(bot, target) && target->GetHealthPercent() < 100)
        {
            needHeals.push_back(target);
        }
    }

    const std::vector<Player*> partyMembers = GetPartyMembers();
    if (partyMembers.empty() && needHeals.empty())
    {
        return nullptr;
    }

    if (!partyMembers.empty() || !needHeals.empty())
    {
        IsTargetOfHealingSpell predicate;
        for (Player* player : partyMembers)
        {
            if (!Check(player) || !sServerFacade.IsAlive(player))
            {
                continue;
            }

            bool isTank = ai->IsTank(player);

            // do not heal dueling members
            if (player->duel && player->duel->opponent)
            {
                continue;
            }

            uint32 incomingDamage = 0;
            if (ai->HasStrategy("preheal", BotState::BOT_STATE_COMBAT))
                incomingDamage = getIncomingdamage(player);

            uint8 health = (((player->GetHealth() - incomingDamage) * 100.0f) / player->GetMaxHealth());
            if (isTank || (health < sPlayerbotAIConfig.almostFullHealth && !IsTargetOfSpellCast(player, predicate)))
            { 
                needHeals.push_back(player);
            }

            Pet* pet = player->GetPet();
            if (pet && CanHealPet(pet))
            {
                health = pet->GetHealthPercent();
                if (health < sPlayerbotAIConfig.almostFullHealth || !IsTargetOfSpellCast(player, predicate))
                {
                    needHeals.push_back(pet);
                }
            }

            if (isTank && bot->IsInGroup(player))
            {
                tankTargets.push_back(player);
            }
        }
    }

    if (needHeals.empty() && tankTargets.empty())
    {
        return nullptr;
    }

    if (needHeals.empty() && !tankTargets.empty())
    {
        needHeals = tankTargets;
    }

    bool preHealing = ai->HasStrategy("preheal", BotState::BOT_STATE_COMBAT);
    sort(needHeals.begin(), needHeals.end(), [preHealing](const Unit* u1, const Unit* u2) { return compareByMissingHealth(u1, u2, preHealing); });

    int healerIndex = 0;
    if (!partyMembers.empty())
    {
        for (Player* player : partyMembers)
        {
            if (!ai->IsSafe(player))
            {
                continue;
            }
            else if (player == bot)
            {
                break;
            }
            else if (ai->IsHeal(player) && player->GetPlayerbotAI())
            {
                float percent = (float)player->GetPower(POWER_MANA) / (float)player->GetMaxPower(POWER_MANA) * 100.0;
                if (percent > sPlayerbotAIConfig.lowMana)
                {
                    healerIndex++;
                }
            }
        }
    }
    else
    {
        healerIndex = 1;
    }

    healerIndex = healerIndex % needHeals.size();
    return needHeals[healerIndex];
}

bool PartyMemberToHeal::CanHealPet(Pet* pet)
{
    return MINI_PET != pet->getPetType();
}

bool PartyMemberToHeal::Check(Unit* player)
{
    bool isBg = bot->InBattleGround();

    float maxDist = ai->GetRange("heal");
    if (isBg)
    {
        maxDist *= 0.5f;
    }

    if (!player)
        return false;

    if (player->GetObjectGuid() == bot->GetObjectGuid())
        return false;

    if (player->GetMapId() != bot->GetMapId())
        return false;

    if (!player->IsInWorld())
        return false;
                                                     
    if (sServerFacade.GetDistance2d(bot, player) > maxDist)
        return false;

    return true;
}

std::vector<Player*> PartyMemberToHeal::GetPartyMembers()
{
    std::vector<Player*> partyMembers;
    if (ai->HasStrategy("focus heal targets", BotState::BOT_STATE_COMBAT))
    {
        Unit* player = nullptr;
        const std::list<ObjectGuid> focusHealTargets = AI_VALUE(std::list<ObjectGuid>, "focus heal targets");
        for(const ObjectGuid& focusHealTarget : focusHealTargets)
        {
            Player* player = (Player*)ai->GetUnit(focusHealTarget);
            if (player && player->IsInGroup(bot) && ai->IsSafe(player))
            {
                partyMembers.push_back(player);
            }
        }
    }
    else
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->getSource();
                if (player && ai->IsSafe(player))
                {
                    partyMembers.push_back(player);
                }
            }
        }
    }

    return partyMembers;
}

Unit* PartyMemberToProtect::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return NULL;

    std::vector<Unit*> needProtect;

    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit)
            continue;

        bool isRanged = false;
        if (unit->AI())
        {
            if (unit->AI()->IsRangedUnit())
                isRanged = true;
        }

        Unit* pVictim = unit->GetVictim();
        if (!pVictim || !pVictim->IsPlayer())
            continue;

        if (pVictim == bot)
            continue;

        if (sServerFacade.GetDistance2d(pVictim, bot) > 30.0f)
            continue;

        float attackDistance = isRanged ? 30.0f : 10.0f;
        if (sServerFacade.GetDistance2d(pVictim, unit) > attackDistance)
            continue;

        if (ai->IsTank((Player*)pVictim) && pVictim->GetHealthPercent() > 10)
            continue;
        else if (pVictim->GetHealthPercent() > 30)
            continue;

        if (find(needProtect.begin(), needProtect.end(), pVictim) == needProtect.end())
        needProtect.push_back(pVictim);
    }

    if (needProtect.empty())
        return NULL;

    sort(needProtect.begin(), needProtect.end(), compareByHealth);

    return needProtect[0];
}

Unit* PartyMemberToRemoveRoots::Calculate()
{
    Unit* target = nullptr;
    Group* group = bot->GetGroup();
    if(group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->getSource();
            if (sServerFacade.IsAlive(player))
            {
                if (player->duel && player->duel->opponent)
                    continue;

                if (player->HasAuraType(SPELL_AURA_MOD_ROOT) || player->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED))
                {
                    if (!ai->HasAura("stealth", player) && !ai->HasAura("prowl", player))
                    {
                        target = player;
                        break;
                    }
                }
            }
        }
    }

    return target;
}
