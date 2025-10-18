
#include "playerbot/playerbot.h"
#include "Stances.h"

#include "playerbot/ServerFacade.h"
#include "Arrow.h"

using namespace ai;

Unit* Stance::GetTarget()
{
    Unit *target = AI_VALUE(Unit*, GetTargetName());
    if (target) return target;

    ObjectGuid attackTarget = context->GetValue<ObjectGuid>("attack target")->Get();
    if (attackTarget) ai->GetUnit(attackTarget);

    return NULL;
}

WorldLocation Stance::GetLocation()
{
    Unit* target = GetTarget();
    if (!target)
        return Formation::NullLocation;

    return GetLocationInternal();
}

WorldLocation Stance::GetNearLocation(float angle, float distance)
{
    Unit* target = GetTarget();

    float x = target->GetPositionX() + cos(angle) * distance,
         y = target->GetPositionY()+ sin(angle) * distance,
         z = target->GetPositionZ();

    if (bot->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true))
        return WorldLocation(bot->GetMapId(), x, y, z);

    return Formation::NullLocation;
}

WorldLocation MoveStance::GetLocationInternal()
{
    Unit* target = GetTarget();
    float distance = std::max(sPlayerbotAIConfig.meleeDistance, target->GetObjectBoundingRadius());

    float angle = GetAngle();
    return GetNearLocation(angle, distance);
}

namespace ai
{
    class NearStance : public MoveStance
    {
    public:
        NearStance(PlayerbotAI* ai) : MoveStance(ai, "near") {}

        virtual float GetAngle()
        {
            Unit* target = GetTarget();

#ifdef MANGOS
            if (target->getVictim() && target->getVictim()->GetObjectGuid() == bot->GetObjectGuid())
                return target->GetOrientation();
#endif
#ifdef CMANGOS
            if (target->GetVictim() && target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid())
                return target->GetOrientation();
#endif

            if (ai->HasStrategy("behind", BotState::BOT_STATE_COMBAT))
            {
                Unit* target = GetTarget();
                Group* group = bot->GetGroup();
                int index = 0, count = 0;
                if (group)
                {
                    for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
                    {
                        Player* member = ref->getSource();
                        if (!ai->IsSafe(member))
                            continue;

                        if (member == bot) index = count;
                        if (member && !ai->IsRanged(member) && !ai->IsTank(member)) count++;
                    }
                }

                float angle = target->GetOrientation() + M_PI;
                if (!count) return angle;

                float increment = M_PI / 4 / count;
                return round((angle + index * increment - M_PI / 4) * 10.0f) / 10.0f;
            }

            float angle = GetFollowAngle() + target->GetOrientation();

            Player* master = GetMaster();
            if (master)
                angle -= master->GetOrientation();

            return angle;
        }
    };

    class TankStance : public MoveStance
    {
    public:
        TankStance(PlayerbotAI* ai) : MoveStance(ai, "tank") {}

        virtual float GetAngle()
        {
            Unit* target = GetTarget();
            return target->GetOrientation();
        }
    };

    class TurnBackStance : public MoveStance
    {
    public:
        TurnBackStance(PlayerbotAI* ai) : MoveStance(ai, "turnback") {}

        virtual float GetAngle()
        {
            Unit* target = GetTarget();
            Group* group = bot->GetGroup();
            int count = 0;
            float angle = 0.0f;
            if (group)
            {
                for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->getSource();
                    if (member && ai->IsSafe(member) && member != bot && ai->IsRanged(member))
                    {
                        angle += target->GetAngle(member);
                        count++;
                    }
                }
            }

            if (!count) return target->GetOrientation();
            return round((angle / count) * 10.0f) / 10.0f + M_PI;
        }
    };

    class BehindStance : public MoveStance
    {
    public:
        BehindStance(PlayerbotAI* ai) : MoveStance(ai, "behind") {}

        virtual float GetAngle()
        {
            Unit* target = GetTarget();
            Group* group = bot->GetGroup();
            int index = 0, count = 0;
            if (group)
            {
                for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->getSource();
                    if (!ai->IsSafe(member))
                        continue;
                    if (member == bot) index = count;
                    if (member && !ai->IsRanged(member) && !ai->IsTank(member)) count++;
                }
            }

            float angle = target->GetOrientation() + M_PI;
            if (!count) return angle;

            float increment = M_PI / 4 / count;
            return round((angle + index * increment - M_PI / 4) * 10.0f) / 10.0f;
        }
    };
};

StanceValue::StanceValue(PlayerbotAI* ai) : ManualSetValue<Stance*>(ai, new NearStance(ai), "stance")
{
}

void StanceValue::Reset()
{
    if (value) delete value;
    value = new NearStance(ai);
}

std::string StanceValue::Save()
{
    return value ? value->getName() : "?";
}

bool StanceValue::Load(std::string name)
{
    if (name == "behind")
    {
        if (value) delete value;
        value = new BehindStance(ai);
    }
    else if (name == "near" || name == "default")
    {
        if (value) delete value;
        value = new NearStance(ai);
    }
    else if (name == "tank")
    {
        if (value) delete value;
        value = new TankStance(ai);
    }
    else if (name == "turnback" || name == "turn")
    {
        if (value) delete value;
        value = new TurnBackStance(ai);
    }
    else return false;

    return true;
}

bool SetStanceAction::Execute(Event& event)
{
    std::string stance = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    StanceValue* value = (StanceValue*)context->GetValue<Stance*>("stance");
    if (stance == "?" || stance.empty())
    {
        std::ostringstream str; str << "Stance: |cff00ff00" << value->Get()->getName();
        ai->TellPlayer(requester, str);
        return true;
    }

    if (stance == "show")
    {
        WorldLocation loc = value->Get()->GetLocation();
        if (!Formation::IsNullLocation(loc))
            ai->Ping(loc.coord_x, loc.coord_y);

        return true;
    }

    if (!value->Load(stance))
    {
        std::ostringstream str; str << "Invalid stance: |cffff0000" << stance;
        ai->TellPlayer(requester, str);
        ai->TellPlayer(requester, "Please set to any of:|cffffffff near (default), tank, turnback, behind");
        return false;
    }

    std::ostringstream str; str << "Stance set to: " << stance;
    ai->TellPlayer(requester, str);
    return true;
}
