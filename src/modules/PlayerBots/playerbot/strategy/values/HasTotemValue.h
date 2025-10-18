#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "playerbot/LootObjectStack.h"

namespace ai
{
    class HasTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HasTotemValue(PlayerbotAI* ai, std::string name = "has totem") : BoolCalculatedValue(ai, name), Qualified() {}

        bool Calculate()
        {
            std::list<ObjectGuid> units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs");
            for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                Creature* creature = dynamic_cast<Creature*>(unit);
                if (!creature || !creature->IsTotem())
                    continue;

                if (strstri(creature->GetName(), qualifier.c_str()) && sServerFacade.GetDistance2d(bot, creature) <= ai->GetRange("spell"))
                    return true;
            }

            return false;
        }
    };
}
