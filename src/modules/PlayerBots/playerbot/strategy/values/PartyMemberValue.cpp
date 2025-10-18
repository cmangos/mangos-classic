
#include "playerbot/playerbot.h"
#include "PartyMemberValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

Unit* PartyMemberValue::FindPartyMember(std::list<Player*>* party, FindPlayerPredicate &predicate, bool ignoreTanks)
{
    for (std::list<Player*>::iterator i = party->begin(); i != party->end(); ++i)
    {
        Player* player = *i;

        if (!player)
            continue;

        if (ignoreTanks && ai->IsTank(player))
            continue;

        if (bot->GetGroup() && !player->IsInGroup(bot) && !AI_VALUE2(bool, "can free move to", GuidPosition(player).to_string())) continue;

        if (Check(player) && predicate.Check(player))
            return player;

        Pet* pet = player->GetPet();
        if (pet && Check(pet) && predicate.Check(pet))
            return pet;
    }

    return NULL;
}

Unit* PartyMemberValue::FindPartyMember(FindPlayerPredicate &predicate, bool ignoreOutOfGroup, bool ignoreTanks)
{
    Player* master = GetMaster();
    std::list<ObjectGuid> nearestPlayers;
    if(ai->AllowActivity(OUT_OF_PARTY_ACTIVITY))
        nearestPlayers = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");      

    std::list<ObjectGuid> nearestGroupPlayers;

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (!ref->getSource() || bot->GetMapId() != ref->getSource()->GetMapId()) continue;

            if (ref->getSource() != bot)
            {
                if (ref->getSubGroup() != bot->GetSubGroup())
                {
                    nearestGroupPlayers.push_back(ref->getSource()->GetObjectGuid());
                }
                else
                {
                    nearestGroupPlayers.push_front(ref->getSource()->GetObjectGuid());
                }
            }
        }
    }
    
    if (!ignoreOutOfGroup && !nearestPlayers.empty() && nearestPlayers.size() < 100  && sServerFacade.IsDistanceLessThan(AI_VALUE2(float, "distance", "master target"), sPlayerbotAIConfig.sightDistance))
        nearestGroupPlayers.insert(nearestGroupPlayers.end(), nearestPlayers.begin(), nearestPlayers.end());

    nearestPlayers = nearestGroupPlayers;

    std::list<Player*> healers, tanks, others, masters;
    if (master) masters.push_back(master);
    for (std::list<ObjectGuid>::iterator i = nearestPlayers.begin(); i != nearestPlayers.end(); ++i)
    {
        Player* player = dynamic_cast<Player*>(ai->GetUnit(*i));
        if (!player || player == bot) 
        {
            continue;
        }

        if (ai->IsHeal(player))
        {
            healers.push_back(player);
        }
        else if (ai->IsTank(player) && !ignoreTanks)
        {
            tanks.push_back(player);
        }
        else if (player != master)
        {
            others.push_back(player);
        }
    }

    std::list<std::list<Player*>* > lists;
    lists.push_back(&healers);
    lists.push_back(&tanks);
    lists.push_back(&masters);
    lists.push_back(&others);

    for (std::list<std::list<Player*>* >::iterator i = lists.begin(); i != lists.end(); ++i)
    {
        std::list<Player*>* party = *i;
        Unit* target = FindPartyMember(party, predicate, ignoreTanks);
        if (target)
            return target;
    }

    if (GuidPosition rpgTarget = AI_VALUE(GuidPosition, "rpg target"))
    {
        Unit* target = rpgTarget.GetCreature(bot->GetInstanceId());

        if (target && sServerFacade.IsFriendlyTo(bot, target) && predicate.Check(target) && AI_VALUE2(bool, "can free move to", GuidPosition(target).to_string()))
           return target;
    }

    return NULL;
}

bool PartyMemberValue::Check(Unit* player)
{
    return player && player != bot && player->GetMapId() == bot->GetMapId() &&
        bot->IsWithinDistInMap(player, sPlayerbotAIConfig.sightDistance, false);
}

bool PartyMemberValue::IsTargetOfSpellCast(Player* target, SpellEntryPredicate &predicate)
{
    std::list<ObjectGuid> nearestPlayers = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");
    ObjectGuid targetGuid = target ? target->GetObjectGuid() : bot->GetObjectGuid();
    ObjectGuid corpseGuid = target && target->GetCorpse() ? target->GetCorpse()->GetObjectGuid() : ObjectGuid();

    for (std::list<ObjectGuid>::iterator i = nearestPlayers.begin(); i != nearestPlayers.end(); ++i)
    {
        Player* player = dynamic_cast<Player*>(ai->GetUnit(*i));
        if (!player || player == bot)
            continue;

        if (player->IsNonMeleeSpellCasted(true))
        {
            for (int type = CURRENT_GENERIC_SPELL; type < CURRENT_MAX_SPELL; type++) 
            {
                Spell* spell = player->GetCurrentSpell((CurrentSpellTypes)type);
                if (spell && predicate.Check(spell->m_spellInfo)) 
                {
                    ObjectGuid unitTarget = spell->m_targets.getUnitTargetGuid();
                    if (unitTarget == targetGuid)
                        return true;

                    if (corpseGuid)
                    {
                        ObjectGuid corpseTarget = spell->m_targets.getCorpseTargetGuid();
                        if (corpseTarget == corpseGuid)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}
