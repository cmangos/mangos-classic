#include "playerbot/playerbot.h"
#include "Formations.h"
#include "Arrow.h"

using namespace ai;

WorldLocation ArrowFormation::GetLocationInternal()
{
    if (!bot->GetGroup()) return Formation::NullLocation;

    Build();

    int tankLines = 1 + tanks.Size() / 6;
    int meleeLines = 1 + melee.Size() / 6;
    int rangedLines = 1 + ranged.Size() / 6;
    int healerLines = 1 + healers.Size() / 6;
    float offset = 0;
    float range = ai->GetRange("follow");

    Unit* followTarget = AI_VALUE(Unit*, "follow target");
    if (!ai->IsSafe(followTarget))
        return Formation::NullLocation;

    float orientation = followTarget->GetOrientation();
    MultiLineUnitPlacer placer(orientation, range);

    tanks.PlaceUnits(&placer);
    tanks.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += tankLines * range;
    melee.PlaceUnits(&placer);
    melee.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += meleeLines * range;
    ranged.PlaceUnits(&placer);
    ranged.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    offset += rangedLines * range;
    healers.PlaceUnits(&placer);
    healers.Move(-cos(orientation) * offset, -sin(orientation) * offset);

    if (!masterUnit || !botUnit)
        return Formation::NullLocation;

    float x = followTarget->GetPositionX() - masterUnit->GetX() + botUnit->GetX();
    float y = followTarget->GetPositionY() - masterUnit->GetY() + botUnit->GetY();
    float z = followTarget->GetPositionZ();

#ifdef MANGOSBOT_TWO
    float ground = followTarget->GetMap()->GetHeight(followTarget->GetPhaseMask(), x, y, z + 0.5f);
#else
    float ground = followTarget->GetMap()->GetHeight(x, y, z + 0.5f);
#endif
    if (ground <= INVALID_HEIGHT)
        return Formation::NullLocation;

    return WorldLocation(followTarget->GetMapId(), x, y, 0.05f + ground);
}

void ArrowFormation::Build()
{
    if (built)
        return;

    FillSlotsExceptMaster();
    AddMasterToSlot();

    built = true;
}

FormationSlot* ArrowFormation::FindSlot(Player* member)
{
    if (ai->IsTank(member))
        return &tanks;
    else if (ai->IsHeal(member))
        return &healers;
    else if (ai->IsRanged(member))
        return &ranged;
    else
        return &melee;
}

void ArrowFormation::FillSlotsExceptMaster()
{
    Unit* followTarget = AI_VALUE(Unit*, "follow target");
    Group* group = bot->GetGroup();
    GroupReference *gref = group->GetFirstMember();
    uint32 index = 0;
    while (gref)
    {
        Player* member = gref->getSource();
        if (ai->IsSafe(member))
        {
            if (member == bot)
                FindSlot(member)->AddLast(botUnit = new FormationUnit(index, false));
            else if (member != followTarget)
                FindSlot(member)->AddLast(new FormationUnit(index, false));
            index++;
        }
        gref = gref->next();
    }
}

void ArrowFormation::AddMasterToSlot()
{
    Unit* followTarget = AI_VALUE(Unit*, "follow target");
    Group* group = bot->GetGroup();
    GroupReference *gref = group->GetFirstMember();
    uint32 index = 0;
    while (gref)
    {
        Player* member = gref->getSource();

        if (member == followTarget)
        {
            FindSlot(member)->InsertAtCenter(masterUnit = new FormationUnit(index, true));
            break;
        }

        gref = gref->next();
        index++;
    }
}

void FormationSlot::PlaceUnits(UnitPlacer* placer)
{
    uint32 index = 0;
    uint32 count = units.size();
    for (std::vector<FormationUnit*>::iterator i = units.begin(); i != units.end(); ++i)
    {
        FormationUnit* unit = *i;
        unit->SetLocation(placer->Place(unit, index, count));
        index++;
    }
}

UnitPosition MultiLineUnitPlacer::Place(FormationUnit *unit, uint32 index, uint32 count)
{
    SingleLineUnitPlacer placer(orientation, range);
    if (count <= 6)
        return placer.Place(unit, index, count);

    int lineNo = index / 6;
    int indexInLine = index % 6;
    int lineSize = std::max(count - lineNo * 6, uint32(6));
    float x = cos(orientation) * range * lineNo;
    float y = sin(orientation) * range * lineNo;
    return placer.Place(unit, indexInLine, lineSize);
}

UnitPosition SingleLineUnitPlacer::Place(FormationUnit *unit, uint32 index, uint32 count)
{
    float angle = orientation - M_PI / 2.0f;
    float x = cos(angle) * range * ((float)index - (float)count / 2);
    float y = sin(angle) * range * ((float)index - (float)count / 2);
    return UnitPosition(x, y);
}

void FormationSlot::Move(float dx, float dy)
{
    for (std::vector<FormationUnit*>::iterator i = units.begin(); i != units.end(); ++i)
    {
        FormationUnit* unit = *i;
        unit->SetLocation(unit->GetX() + dx, unit->GetY() + dy);
    }
}

FormationSlot::~FormationSlot()
{
    for (std::vector<FormationUnit*>::iterator i = units.begin(); i != units.end(); ++i)
    {
        FormationUnit* unit = *i;
        delete unit;
    }
    units.clear();
}
