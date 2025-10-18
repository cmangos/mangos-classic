
#include "playerbot/playerbot.h"
#include "AttackerCountValues.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

uint8 MyAttackerCountValue::Calculate()
{
    return bot->getAttackers().size();
}

bool HasAggroValue::Calculate()
{
    Unit* target = GetTarget();

    // Player targets don't have aggro
    if (target && !target->IsPlayer())
    {
        HostileReference* ref = sServerFacade.GetHostileRefManager(bot).getFirst();
        if (ref)
        {
            while (ref)
            {
                ThreatManager* threatManager = ref->getSource();
                Unit* attacker = threatManager->getOwner();
                Unit* victim = attacker->GetVictim();

                if ((victim == bot) && (target == attacker))
                {
                    return true;
                }
                else
                {
                    ref = ref->next();
                }
            }

            ref = sServerFacade.GetThreatManager(target).getCurrentVictim();
            if (ref)
            {
                Unit* victim = ref->getTarget();
                return victim && (victim == bot);

                /*
                // What is the purpose of this?
                if (victim)
                {
                    Player* pl = dynamic_cast<Player*>(victim);
                    if (pl && ai->IsTank(pl)) return true;
                }
                */
            }
        }
    }

    return false;
}

uint8 AttackersCountValue::Calculate()
{
    return context->GetValue<std::list<ObjectGuid>>("attackers")->Get().size();
}

uint8 PossibleAttackTargetsCountValue::Calculate()
{
    return context->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get().size();
}

bool HasAttackersValue::Calculate()
{
    return !context->GetValue<std::list<ObjectGuid>>("attackers", 1)->Get().empty();
}

bool HasPossibleAttackTargetsValue::Calculate()
{
    return !context->GetValue<std::list<ObjectGuid>>("possible attack targets", 1)->Get().empty();
}

uint8 BalancePercentValue::Calculate()
{
    float playerLevel = 0,
        attackerLevel = 0;

    playerLevel += bot->GetLevel();

    if (!bot->InBattleGround())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player* player = sObjectMgr.GetPlayer(itr->guid);
                if (!player || !sServerFacade.IsAlive(player) || !player->IsInWorld() || player->GetMapId() != bot->GetMapId() || sServerFacade.GetDistance2d(bot, player) > 30.0f)
                    continue;

                playerLevel += player->GetLevel();
            }
        }
    }

    std::list<ObjectGuid> v = context->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = v.begin(); i!=v.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        if (unit->IsPlayer())
        {
            attackerLevel += unit->GetLevel() * 3;
        }
        else
        {
            Creature* creature = ai->GetCreature((*i));
            if (!creature || !sServerFacade.IsAlive(creature))
                continue;

            uint32 level = creature->GetLevel();
            switch (creature->GetCreatureInfo()->Rank) 
            {
                case CREATURE_ELITE_RARE:
                {
                    level *= 2;
                    break;
                }

                case CREATURE_ELITE_ELITE:
                {
                    level *= 3;
                    break;
                }

                case CREATURE_ELITE_RAREELITE:
                {
                    level *= 3;
                    break;
                }

                case CREATURE_ELITE_WORLDBOSS:
                {
                    level *= 5;
                    break;
                }
            }

            attackerLevel += level;
        }
    }

    Unit* enemy = AI_VALUE(Unit*, "enemy player target");
    if (enemy)
        attackerLevel += enemy->GetLevel() * 3;

    if (!attackerLevel)
        return 100;

    float percent = playerLevel * 100 / attackerLevel;
    return percent <= 200 ? (uint8)percent : 200;
}