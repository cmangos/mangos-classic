
#include "playerbot/playerbot.h"
#include "GreetAction.h"
#include "playerbot/PlayerbotAI.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
using namespace ai;

GreetAction::GreetAction(PlayerbotAI* ai) : Action(ai, "greet")
{
}

bool GreetAction::Execute(Event& event)
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "new player nearby");
    if (!guid || !guid.IsPlayer()) return false;

    Player* player = dynamic_cast<Player*>(ai->GetUnit(guid));
    if (!player) return false;

    if (!sServerFacade.IsInFront(bot, player, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
        sServerFacade.SetFacingTo(bot, player);

    ObjectGuid oldSel = bot->GetSelectionGuid();
    bot->SetSelectionGuid(guid);
    //bot->HandleEmote(EMOTE_ONESHOT_WAVE);
    ai->PlayEmote(TEXTEMOTE_HELLO);
    bot->SetSelectionGuid(oldSel);

    std::set<ObjectGuid>& alreadySeenPlayers = ai->GetAiObjectContext()->GetValue<std::set<ObjectGuid>& >("already seen players")->Get();
    alreadySeenPlayers.insert(guid);

    std::list<ObjectGuid> nearestPlayers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get();
    for (std::list<ObjectGuid>::iterator i = nearestPlayers.begin(); i != nearestPlayers.end(); ++i) {
        alreadySeenPlayers.insert(*i);
    }
    return true;
}
