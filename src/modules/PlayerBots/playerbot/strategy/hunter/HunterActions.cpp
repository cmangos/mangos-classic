
#include "playerbot/playerbot.h"
#include "playerbot/strategy/actions/GenericActions.h"
#include "HunterActions.h"

using namespace ai;

bool CastSerpentStingAction::isUseful()
{
    return CastRangedDebuffSpellAction::isUseful() && AI_VALUE2(uint8, "health", GetTargetName()) > 50 && !(AI_VALUE2(uint8, "mana", GetTargetName()) >= 10);
}

bool CastViperStingAction::isUseful()
{
    return CastRangedDebuffSpellAction::isUseful() && AI_VALUE2(uint8, "mana", GetTargetName()) >= 10;
}

bool FeedPetAction::Execute(Event& event)
{
    Pet* pet = bot->GetPet();
    if (pet && pet->getPetType() == HUNTER_PET && pet->GetHappinessState() != HAPPY)
        pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);

    return true;
}

bool CastAutoShotAction::isUseful()
{
    if (ai->IsInVehicle() && !ai->IsInVehicle(false, false, true))
        return false;

    return ai->HasStrategy("ranged", BotState::BOT_STATE_COMBAT) && AI_VALUE(uint32, "active spell") != AI_VALUE2(uint32, "spell id", getName());
}

bool CastAutoShotAction::Execute(Event& event)
{
    if (!bot->IsStopped())
    {
        ai->StopMoving();
    }

    return CastSpellAction::Execute(event);
}

bool CastSteadyShotAction::Execute(Event& event)
{
    if (CastSpellAction::Execute(event))
    {
        const Item* equippedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (equippedWeapon)
        {
            const ItemPrototype* itemPrototype = equippedWeapon->GetProto();
            weaponDelay = itemPrototype->Delay + sPlayerbotAIConfig.globalCoolDown;
            SetDuration(weaponDelay);
        }

        return true;
    }

    return false;
}
