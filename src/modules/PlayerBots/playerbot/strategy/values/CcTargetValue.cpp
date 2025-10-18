
#include "playerbot/playerbot.h"
#include "CcTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Action.h"

using namespace ai;

class FindTargetForCcStrategy : public FindTargetStrategy
{
public:
    FindTargetForCcStrategy(PlayerbotAI* ai, std::string spell) : FindTargetStrategy(ai)
    {
        this->spell = spell;
        maxDistance = 0;
    }

public:
    virtual void CheckAttacker(Unit* creature, ThreatManager* threatManager)
    {
        Player* bot = ai->GetBot();

        AiObjectContext* context = ai->GetAiObjectContext();

        if (!ai->CanCastSpell(spell, creature, true, nullptr, false, true))
            return;

        if (AI_VALUE(Unit*,"rti cc target") == creature)
        {
            result = creature;
            return;
        }

        if (AI_VALUE(Unit*,"current target") == creature)
            return;

        if (AI_VALUE(Unit*,"rti target") == creature)
            return;

        uint8 health = creature->GetHealthPercent();
        if (health < sPlayerbotAIConfig.mediumHealth)
            return;

        float minDistance = ai->GetRange("spell");
        Group* group = bot->GetGroup();
        if (!group)
            return;

        if (AI_VALUE(uint8,"aoe count") > 2)
        {
            WorldLocation aoe = AI_VALUE(WorldLocation,"aoe position");
            if (sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(creature, aoe.coord_x, aoe.coord_y), sPlayerbotAIConfig.aoeRadius))
                return;
        }

        if (creature->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE) && !(spell == "fear" || spell == "banish"))
            return;

        if (!creature->IsPlayer())
        {
            int tankCount, dpsCount;
            GetPlayerCount(creature, &tankCount, &dpsCount);
            if (!tankCount || !dpsCount)
            {
                result = creature;
                return;
            }
        }

        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *member = sObjectMgr.GetPlayer(itr->guid);
            if(!member || !sServerFacade.IsAlive(member) || member == bot || bot->GetMapId() != member->GetMapId())
                continue;

            if (!ai->IsTank(member))
                continue;

            float distance = sServerFacade.GetDistance2d(member, creature);
            if (distance < minDistance)
                minDistance = distance;
        }

        if ((!result && !creature->IsPlayer()) || minDistance > maxDistance)
        {
            result = creature;
            maxDistance = minDistance;
        }
    }

private:
    std::string spell;
    float maxDistance;
};

Unit* CcTargetValue::Calculate()
{
    std::list<ObjectGuid> possible = AI_VALUE(std::list<ObjectGuid>,"possible targets no los");

    for (std::list<ObjectGuid>::iterator i = possible.begin(); i != possible.end(); ++i)
    {
        ObjectGuid guid = *i;
        Unit* add = ai->GetUnit(guid);
        if (!add)
            continue;

        if (!ai->IsSafe(add))
            continue;

        if (ai->HasMyAura(qualifier, add))
            return NULL;

        if (qualifier == "polymorph")
        {
            if (ai->HasMyAura("polymorph: pig", add))
                return NULL;
            if (ai->HasMyAura("polymorph: turtle", add))
                return NULL;
        }
    }

    FindTargetForCcStrategy strategy(ai, qualifier);
    return FindTarget(&strategy);
}
