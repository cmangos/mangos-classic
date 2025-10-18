
#include "playerbot/playerbot.h"
#include "SnareTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "MotionGenerators/TargetedMovementGenerator.h"

using namespace ai;

Unit* SnareTargetValue::Calculate()
{
    std::string spell = qualifier;

    Unit* enemy = AI_VALUE(Unit*, "enemy player target");
    if (enemy)
    {
        Player* plr = dynamic_cast<Player*>(enemy);
        if (plr && !(plr->HasAuraType(SPELL_AURA_MOD_ROOT) || plr->HasAuraType(SPELL_AURA_MOD_STUN)) && (!plr->IsStopped() || plr->IsNonMeleeSpellCasted(false) || (plr->GetVictim() && plr->GetVictim()->GetObjectGuid() == bot->GetObjectGuid())))
            return enemy;
    }

    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit)
            continue;

        if (sServerFacade.GetDistance2d(bot, unit) > ai->GetRange("spell"))
            continue;

        // case real player or bot not moving
        if (unit->IsPlayer() && unit->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
        {
            if (Unit* victim = unit->GetVictim())
            {
                // check if need to snare
                bool shouldSnare = true;

                // do not slow down if bot is melee and mob/bot attack each other
                if (victim == bot || victim->IsPlayer())
                    shouldSnare = true;

                if (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasAuraType(SPELL_AURA_MOD_STUN))
                    shouldSnare = false;

                if (victim && shouldSnare)
                    return unit;
            }
        }

        Unit* chaseTarget;
        switch (unit->GetMotionMaster()->GetCurrentMovementGeneratorType())
        {
        case FLEEING_MOTION_TYPE:
            return unit;
        case CHASE_MOTION_TYPE:
            chaseTarget = sServerFacade.GetChaseTarget(unit);
            if (!chaseTarget) continue;
            Player* chaseTargetPlayer = sObjectMgr.GetPlayer(chaseTarget->GetObjectGuid());
            
            // check if need to snare
            bool shouldSnare = true;

            // do not slow down if bot is melee and mob/bot attack each other
            if (chaseTargetPlayer && !ai->IsRanged(bot) && chaseTargetPlayer == bot)
                shouldSnare = false;

            if (!sServerFacade.isMoving(unit))
                shouldSnare = false;

            if (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasAuraType(SPELL_AURA_MOD_STUN))
                shouldSnare = false;

            if (chaseTargetPlayer && shouldSnare && !ai->IsTank(chaseTargetPlayer))
                return unit;
        }
    }

    return NULL;
}
