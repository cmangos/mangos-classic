
#include "playerbot/playerbot.h"
#include "TellTargetAction.h"

#include "playerbot/ServerFacade.h"
#include "Combat/ThreatManager.h"

using namespace ai;

bool TellTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (target)
    {
        std::ostringstream out;
		out << "Attacking " << target->GetName();
        ai->TellPlayer(requester, out);

        context->GetValue<Unit*>("old target")->Set(target);
    }
    return true;
}

bool TellAttackersAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    ai->TellPlayer(requester, "--- Attackers ---");

    std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("attackers")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        ai->TellPlayer(requester, unit->GetName());
    }

    ai->TellPlayer(requester, "--- Threat ---");
    HostileReference* ref = sServerFacade.GetHostileRefManager(bot).getFirst();
    if (!ref)
        return true;

    while (ref)
    {
        ThreatManager* threatManager = ref->getSource();
        Unit* unit = threatManager->getOwner();
        float threat = ref->getThreat();

        std::ostringstream out; out << unit->GetName() << " (" << threat << ")";
        ai->TellPlayer(requester, out);

        ref = ref->next();
    }
    return true;
}

bool TellPossibleAttackTargetsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    ai->TellPlayer(requester, "--- Attack Targets ---");

    std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        ai->TellPlayer(requester, unit->GetName());
    }

    ai->TellPlayer(requester, "--- Threat ---");
    HostileReference *ref = sServerFacade.GetHostileRefManager(bot).getFirst();
    if (!ref)
        return true;

    while( ref )
    {
        ThreatManager *threatManager = ref->getSource();
        Unit *unit = threatManager->getOwner();
        float threat = ref->getThreat();

        std::ostringstream out; out << unit->GetName() << " (" << threat << ")";
        ai->TellPlayer(requester, out);

        ref = ref->next();
    }
    return true;
}
