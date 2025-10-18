#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "playerbot/LootObjectStack.h"
#include "playerbot/ServerFacade.h"
#include "PositionValue.h"
#include "Stances.h"

namespace ai
{
    class DistanceValue : public FloatCalculatedValue, public Qualified
	{
	public:
        DistanceValue(PlayerbotAI* ai, std::string name = "distance") : FloatCalculatedValue(ai, name), Qualified() {}

    public:
        float Calculate()
        {
            if (qualifier == "loot target")
            {
                LootObject loot = AI_VALUE(LootObject, qualifier);
                if (loot.IsEmpty())
                    return 0.0f;

                WorldObject* obj = loot.GetWorldObject(bot);
                if (!obj)
                    return 0.0f;

                return sServerFacade.GetDistance2d(ai->GetBot(), obj);
            }

            if (qualifier.find("position_") == 0)
            {
                std::string position = qualifier.substr(9);
                ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()[position];
                if (!pos.isSet()) return 0.0f;
                if (ai->GetBot()->GetMapId() != pos.mapId) return 0.0f;
                return sServerFacade.GetDistance2d(ai->GetBot(), pos.x, pos.y);
            }

            Unit* target = NULL;
            if (qualifier == "rpg target")
            {
                GuidPosition rpgTarget = AI_VALUE(GuidPosition, qualifier);
                if (!rpgTarget) return FLT_MAX;

                WorldPosition oldLocation = rpgTarget;

                rpgTarget.updatePosition(bot->GetInstanceId());

                if(oldLocation != rpgTarget)
                    SET_AI_VALUE(GuidPosition, "rpg target", rpgTarget);

                return rpgTarget.distance(bot);
            }
            else if (qualifier == "travel target")
            {
                TravelTarget * travelTarget = AI_VALUE(TravelTarget *, qualifier);
                return travelTarget->Distance(ai->GetBot());
            }
            else if (qualifier == "last long move")
            {
                WorldPosition target = AI_VALUE(WorldPosition, qualifier);
                return target.distance(ai->GetBot());
            }
            else if (qualifier == "home bind")
            {
                WorldPosition target = AI_VALUE(WorldPosition, qualifier);
                return target.distance(ai->GetBot());
            }
            else if (qualifier == "current target")
            {
                Stance* stance = AI_VALUE(Stance*, "stance");
                WorldLocation loc = stance->GetLocation();
                return sServerFacade.GetDistance2d(ai->GetBot(), loc.coord_x, loc.coord_y);
            }
            else
            {
                target = AI_VALUE(Unit*, qualifier);
                if (target && target == GetMaster() && target != bot)
                {
                    Formation* formation = AI_VALUE(Formation*, "formation");
                    WorldLocation loc = formation->GetLocation();
                    if (Formation::IsNullLocation(loc))
                        loc = WorldLocation(target->GetMapId(), target->GetPosition());

                    return sServerFacade.GetDistance2d(ai->GetBot(), loc.coord_x, loc.coord_y);
                }
            }

            if (!target || !target->IsInWorld())
                return 0.0f;

            if (target == ai->GetBot())
                return 0.0f;

            return sServerFacade.GetDistance2d(ai->GetBot(), target);
        }
    };

    class InsideTargetValue : public BoolCalculatedValue, public Qualified
    {
    public:
        InsideTargetValue(PlayerbotAI* ai, std::string name = "inside target") : BoolCalculatedValue(ai, name), Qualified() {}

    public:
        bool Calculate()
        {
            Unit* target = AI_VALUE(Unit*, qualifier);

            if (!target || !target->IsInWorld() || target == ai->GetBot())
                return false;

            float dist = sServerFacade.GetDistance2d(ai->GetBot(), target->GetPositionX(), target->GetPositionY());
            return sServerFacade.IsDistanceLessThan(dist, target->GetObjectBoundingRadius());
        }
    };
}
