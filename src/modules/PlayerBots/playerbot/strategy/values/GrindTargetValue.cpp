
#include "playerbot/playerbot.h"
#include "GrindTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/ServerFacade.h"
#include "AttackersValue.h"
#include "PossibleAttackTargetsValue.h"
#include "playerbot/strategy/actions/ChooseTargetActions.h"
#include "Tools/Formulas.h"

using namespace ai;

Unit* GrindTargetValue::Calculate()
{
    uint32 memberCount = 1;
    Group* group = bot->GetGroup();
    if (group)
        memberCount = group->GetMembersCount();

    Unit* target = NULL;
    uint32 assistCount = 0;
    while (!target && assistCount < memberCount)
    {
        target = FindTargetForGrinding(assistCount++);
    }

    return target;
}

Unit* GrindTargetValue::FindTargetForGrinding(int assistCount)
{
    uint32 memberCount = 1;
    Group* group = bot->GetGroup();
    Player* master = GetMaster();

    if (master && (master == bot || master->GetMapId() != bot->GetMapId() || master->IsBeingTeleported() || !master->GetPlayerbotAI()))
        master = nullptr;

    std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        if (!bot->InBattleGround() && !AI_VALUE2(bool, "can free target", GuidPosition(unit).to_string())) //Do not grind mobs far away from master.
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + "(hostile) ignored (out of free range).");
            continue;
        }

        if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) +"(hostile) selected.");
       
        return unit;
    }

    std::list<ObjectGuid> targets = *context->GetValue<std::list<ObjectGuid> >("possible targets");

    if (targets.empty())
        return NULL;

    float distance = 0;
    Unit* result = NULL;

    std::unordered_map<uint32, bool> needForQuestMap;

    for (std::list<ObjectGuid>::iterator tIter = targets.begin(); tIter != targets.end(); tIter++)
    {
        Unit* unit = ai->GetUnit(*tIter);
        if (!unit)
            continue;

#ifdef MANGOSBOT_TWO 
        if (bot->GetMapId() == 609)
        {
            switch (unit->GetEntry())
            {
            case 28605: //Havenshire Stallion
            case 28606: //Havenshire Mare
            case 28607: //Havenshire Cotl
            case 28891: //Scarlet Miner
            case 28819: //Scarlet Miner
            case 28769: //Shadowy tormentor
            case 28768: //Dark Rider of Archeus
                continue;
            case 29080: //Scarlet Champion
            case 29029: //Scarlet Inquisitor
            case 29000: //Scarlet Commander Rodrick
            case 28940: //Scarlet Crusader
            case 28939: //Scarlet Preacher
            case 28936: //Scarlet Commander
            case 28898: //Scarlet Captain
            case 28896: //Scarlet Infantryman
            case 28895: //Scarlet Medic
            case 28892: //Scarlet Peasant
            case 28610: //Scarlet Marksman
                if (AI_VALUE2(bool, "need quest objective", "12680,0"))
                    continue;
                if (AI_VALUE2(bool, "need quest objective", "12701"))
                    continue;
                if (AI_VALUE2(bool, "need quest objective", "12727"))
                    continue;
                if (AI_VALUE2(bool, "need quest objective", "12754"))
                    continue;                
                break;
            case 29102: //HearthglenCrusader
            case 29103: //Tirisfal Crusader
            case 29104: //Scarlet Ballista
                if (AI_VALUE2(bool, "need quest objective", "12779"))
                {
                    if (AI_VALUE2(bool, "trigger active", "in vehicle") && bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance))
                        return unit;
                    continue;
                }
                break;
            case 28834: //Scarlet Fleet Defender
            case 28850: //Scarlet Land Canon
            case 28856: //Scarlet Fleet Guardian
                if (AI_VALUE2(bool, "need quest objective", "12701,0"))
                {
                    if (AI_VALUE2(bool, "trigger active", "in vehicle::Scarlet Cannon") && !urand(0, 5))
                        return unit;
                    continue;
                }
            }
        }
#endif

        if (abs(bot->GetPositionZ() - unit->GetPositionZ()) > sPlayerbotAIConfig.spellDistance)
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (to far above/below).");
            continue;
        }

        if (!bot->InBattleGround() && !AI_VALUE2(bool, "can free target", GuidPosition(unit).to_string())) //Do not grind mobs far away from master.
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (out of free range).");
            continue;
        }

        if (!bot->InBattleGround() && master && ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && sServerFacade.GetDistance2d(master, unit) > sPlayerbotAIConfig.proximityDistance)
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (far from master).");
            continue;
        }

        if (!bot->InBattleGround() && (int)unit->GetLevel() - (int)bot->GetLevel() > 4 && !unit->GetObjectGuid().IsPlayer())
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (" + std::to_string((int)unit->GetLevel() - (int)bot->GetLevel()) + " levels above bot).");
            continue;
        }

        Creature* creature = dynamic_cast<Creature*>(unit);
        if (creature && creature->GetCreatureInfo() && creature->GetCreatureInfo()->Rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite") &&
            !AI_VALUE2(bool, "trigger active", "in vehicle"))
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (can not fight elites currently).");
            continue;
        }

        if (!AttackersValue::IsValid(unit, bot, nullptr, false, false))
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (is pet or evading/unkillable).");
            continue;
        }

        if (!PossibleAttackTargetsValue::IsPossibleTarget(unit, bot, sPlayerbotAIConfig.sightDistance, false))
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (tapped, cced or out of range).");
            continue;
        }

        if (creature && creature->IsCritter() && urand(0, 10))
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (ignore critters).");
            continue;
        }

        float newdistance = sServerFacade.GetDistance2d(bot, unit);
       
        if (unit->GetEntry() && !AI_VALUE2(bool, "need for quest", std::to_string(unit->GetEntry())))
        {
            if (urand(0, 100) < 99 && AI_VALUE(bool, "travel target working") && typeid(AI_VALUE(TravelTarget*, "travel target")->GetDestination()) != typeid(GrindTravelDestination))
            {
                if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (not needed for active quest).");

                continue;
            }
            else if (creature && !MaNGOS::XP::Gain(bot, creature) && urand(0, 50))
            {
                if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                    if ((AI_VALUE(bool, "travel target traveling") && typeid(AI_VALUE(TravelTarget*, "travel target")->GetDestination()) != typeid(GrindTravelDestination)))
                        ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " ignored (not xp and not needed for quest).");

                continue;
            }
            else if (urand(0, 100) < 75)
            {
                if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                    if (AI_VALUE(bool, "travel target traveling") && typeid(AI_VALUE(TravelTarget*, "travel target")->GetDestination()) != typeid(GrindTravelDestination))
                        ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " increased distance (not needed for quest).");

                newdistance += 20;
            }
        }

        if (!bot->InBattleGround() && GetTargetingPlayerCount(unit) > assistCount)
        {
            if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayer(GetMaster(), chat->formatWorldobject(unit) + " increased distance (" + std::to_string(GetTargetingPlayerCount(unit)) + " bots already targeting).");

            newdistance =+ GetTargetingPlayerCount(unit) * 5;
        }

        if (group)
        {
            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player* member = sObjectMgr.GetPlayer(itr->guid);
                if (!member || !sServerFacade.IsAlive(member))
                    continue;

                newdistance = sServerFacade.GetDistance2d(member, unit);
                if (!result || newdistance < distance)
                {
                    distance = newdistance;
                    result = unit;
                }
            }
        }
        else
        {
            if (!result || (newdistance < distance && urand(0, abs(distance - newdistance)) > sPlayerbotAIConfig.sightDistance * 0.1))
            {
                distance = newdistance;
                result = unit;
            }
        }
    }

    if (ai->HasStrategy("debug grind", BotState::BOT_STATE_NON_COMBAT))
    {
        if(result)
        {
            ai->TellPlayer(GetMaster(), chat->formatWorldobject(result) + " selected.");
        }
        else
        {
            ai->TellPlayer(GetMaster(), "No grind target found.");
        }
    }

    return result;
}

int GrindTargetValue::GetTargetingPlayerCount( Unit* unit )
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    int count = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player *member = sObjectMgr.GetPlayer(itr->guid);
        if( !member || !sServerFacade.IsAlive(member) || member == bot)
            continue;

        PlayerbotAI* ai = member->GetPlayerbotAI();
        if ((ai && *ai->GetAiObjectContext()->GetValue<Unit*>("current target") == unit) ||
            (!ai && member->GetSelectionGuid() == unit->GetObjectGuid()))
            count++;
    }

    return count;
}