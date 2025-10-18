#pragma once
#include "GenericSpellActions.h"
#include "ReachTargetActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "RemoveAuraAction.h"

namespace ai
{
    class MeleeAction : public AttackAction 
    {
    public:
        MeleeAction(PlayerbotAI* ai) : AttackAction(ai, "melee") {}
        virtual std::string GetTargetName() { return "current target"; }
        virtual bool isUseful();
    };

    class UseLightwellAction : public MovementAction
    {
    public:
        UseLightwellAction(PlayerbotAI* ai) : MovementAction(ai, "lightwell") {}
        ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }

        bool isUseful() override
        {
            if (MovementAction::isUseful())
            {
                return (bot->getClass() == CLASS_PRIEST || bot->GetGroup()) && bot->GetHealthPercent() < sPlayerbotAIConfig.mediumHealth && !ai->HasAura("lightwell renew", bot);
            }

            return false;
        }

        bool isPossible() override
        {
            std::list<ObjectGuid> closeObjects = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
            if (closeObjects.empty())
                return false;

            for (std::list<ObjectGuid>::iterator i = closeObjects.begin(); i != closeObjects.end(); ++i)
            {
                GameObject* go = ai->GetGameObject(*i);
                if (!go)
                    continue;

                if (!(go->GetEntry() == 181106 || go->GetEntry() == 181165 || go->GetEntry() == 181102 || go->GetEntry() == 181105))
                    continue;

                if (!sServerFacade.isSpawned(go) || go->GetGoState() != GO_STATE_READY || !bot->CanInteract(go))
                    continue;

                if (Unit* owner = go->GetOwner())
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        Player* ownerPlayer = (Player*)owner;
                        if (!ownerPlayer)
                            return false;

                        if (!ownerPlayer->IsInGroup(bot))
                            continue;
                    }
                }

                lightwellGameObject = *i;
                return MovementAction::isPossible();
            }

            return false;
        }
        
        virtual bool Execute(Event& event) override
        {
            GameObject* go = ai->GetGameObject(lightwellGameObject);
            if (go)
            {
                if (bot->IsWithinDistInMap(go, INTERACTION_DISTANCE, false))
                {
                    WorldPacket data(CMSG_GAMEOBJ_USE);
                    data << go->GetObjectGuid();
                    bot->GetSession()->HandleGameObjectUseOpcode(data);
                    return true;
                }
                else
                {
                    return MoveNear(go, 4.0f);
                }
            }

            return false;
        }

    private:
        ObjectGuid lightwellGameObject;
    };

    class ChatCommandAction : public Action
    {
    public:
        ChatCommandAction(PlayerbotAI* ai, std::string name, uint32 duration = sPlayerbotAIConfig.reactDelay) : Action(ai, name, duration) {}
    public:
        virtual bool Execute(Event& event) override { return true; }
    };

    class UpdateStrategyDependenciesAction : public Action
    {
        struct StrategyToUpdate
        {
            StrategyToUpdate(BotState inState, std::string inStrategy, std::vector<std::string> inStrategiesRequired = {})
            : state(inState)
            , name(inStrategy)
            , strategiesRequired(inStrategiesRequired) {}

            BotState state;
            std::string name;
            std::vector<std::string> strategiesRequired;
        };

     public:
         UpdateStrategyDependenciesAction(PlayerbotAI* ai, std::string name = "update strategy dependencies") : Action(ai, name) {}
         bool Execute(Event& event) override;
         bool isUseful() override;

    protected:
        std::vector<StrategyToUpdate> strategiesToUpdate;

    private:
        std::vector<const StrategyToUpdate*> strategiesToAdd;
        std::vector<const StrategyToUpdate*> strategiesToRemove;
    };

    class RemoveBlessingOfSalvationAction : public RemoveAuraAction
    {
    public:
        RemoveBlessingOfSalvationAction(PlayerbotAI* ai) : RemoveAuraAction(ai, "blessing of salvation") {}
    };

    class RemoveGreaterBlessingOfSalvationAction : public RemoveAuraAction
    {
    public:
        RemoveGreaterBlessingOfSalvationAction(PlayerbotAI* ai) : RemoveAuraAction(ai, "greater blessing of salvation") {}
    };

    class InitializePetAction : public Action
    {
    public:
        InitializePetAction(PlayerbotAI* ai) : Action(ai, "initialize pet") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    class SetPetAction : public Action
    {
    public:
        SetPetAction(PlayerbotAI* ai) : Action(ai, "pet") {}
        bool Execute(Event& event) override;
    };
}
