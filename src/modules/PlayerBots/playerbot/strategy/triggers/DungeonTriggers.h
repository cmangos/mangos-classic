#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class EnterDungeonTrigger : public Trigger
    {
    public:
        // You can get the mapID from worlddb > instance_template > map column
        // or from here https://wow.tools/dbc/?dbc=map&build=1.12.1.5875
        EnterDungeonTrigger(PlayerbotAI* ai, std::string name, std::string dungeonStrategy, uint32 mapID)
        : Trigger(ai, name, 5)
        , dungeonStrategy(dungeonStrategy)
        , mapID(mapID) {}

        bool IsActive() override;

    private:
        std::string dungeonStrategy;
        uint32 mapID;
    };

    class LeaveDungeonTrigger : public Trigger
    {
    public:
        // You can get the mapID from worlddb > instance_template > map column
        // or from here https://wow.tools/dbc/?dbc=map&build=1.12.1.5875
        LeaveDungeonTrigger(PlayerbotAI* ai, std::string name, std::string dungeonStrategy, uint32 mapID)
        : Trigger(ai, name, 5)
        , dungeonStrategy(dungeonStrategy)
        , mapID(mapID) {}

        bool IsActive() override;

    private:
        std::string dungeonStrategy;
        uint32 mapID;
    };

    class StartBossFightTrigger : public Trigger
    {
    public:
        StartBossFightTrigger(PlayerbotAI* ai, std::string name, std::string bossStrategy, uint64 bossID)
        : Trigger(ai, name, 1)
        , bossStrategy(bossStrategy)
        , bossID(bossID) {}

        bool IsActive() override;

    private:
        std::string bossStrategy;
        uint64 bossID;
    };

    class EndBossFightTrigger : public Trigger
    {
    public:
        EndBossFightTrigger(PlayerbotAI* ai, std::string name, std::string bossStrategy, uint64 bossID)
        : Trigger(ai, name, 5)
        , bossStrategy(bossStrategy)
        , bossID(bossID) {}

        bool IsActive() override;

    private:
        std::string bossStrategy;
        uint64 bossID;
    };

    class CloseToHazardTrigger : public Trigger
    {
    public:
        CloseToHazardTrigger(PlayerbotAI* ai, std::string name, int checkInterval, float hazardRadius, time_t hazardDuration)
        : Trigger(ai, name, checkInterval)
        , hazardRadius(hazardRadius)
        , hazardDuration(hazardDuration) {}

        bool IsActive() override final;

    protected:
        virtual std::list<ObjectGuid> GetPossibleHazards() = 0;
        virtual bool IsHazardValid(const ObjectGuid& hazzardGuid);

    private:
        float GetDistanceToHazard(const ObjectGuid& hazzardGuid);

    protected:
        float hazardRadius;
        time_t hazardDuration;
    };

    class CloseToGameObjectHazardTrigger : public CloseToHazardTrigger
    {
    public:
        CloseToGameObjectHazardTrigger(PlayerbotAI* ai, std::string name, uint32 gameObjectID, float radius, time_t expirationTime)
        : CloseToHazardTrigger(ai, name, 1, radius, expirationTime)
        , gameObjectID(gameObjectID) {}

    private:
        std::list<ObjectGuid> GetPossibleHazards() override;

    private:
        uint32 gameObjectID;
    };

    class CloseToCreatureHazardTrigger : public CloseToHazardTrigger
    {
    public:
        CloseToCreatureHazardTrigger(PlayerbotAI* ai, std::string name, uint32 creatureID, float radius, time_t expirationTime)
        : CloseToHazardTrigger(ai, name, 1, radius, expirationTime)
        , creatureID(creatureID) {}

    private:
        std::list<ObjectGuid> GetPossibleHazards() override;
        bool IsHazardValid(const ObjectGuid& hazzardGuid) override;

    protected:
        uint32 creatureID;
    };

    class CloseToHostileCreatureHazardTrigger : public CloseToCreatureHazardTrigger
    {
    public:
        CloseToHostileCreatureHazardTrigger(PlayerbotAI* ai, std::string name, uint32 creatureID, float radius, time_t expirationTime)
        : CloseToCreatureHazardTrigger(ai, name, creatureID, radius, expirationTime) {}

    private:
        std::list<ObjectGuid> GetPossibleHazards() override;
    };

    class CloseToCreatureTrigger : public Trigger
    {
    public:
        CloseToCreatureTrigger(PlayerbotAI* ai, std::string name, uint32 creatureID, float range)
        : Trigger(ai, name, 1)
        , creatureID(creatureID)
        , range(range) {}

        bool IsActive() override;

    private:
        uint32 creatureID;
        float range;
    };

    class ItemReadyTrigger : public Trigger
    {
    public:
        ItemReadyTrigger(PlayerbotAI* ai, std::string name, uint32 itemID)
        : Trigger(ai, name, 1)
        , itemID(itemID) {}

        virtual bool IsActive() override;

    protected:
        uint32 itemID;
    };

    class ItemBuffReadyTrigger : public ItemReadyTrigger
    {
    public:
        ItemBuffReadyTrigger(PlayerbotAI* ai, std::string name, uint32 itemID, uint32 buffID)
        : ItemReadyTrigger(ai, name, itemID)
        , buffID(buffID) {}

        bool IsActive() override;

    private:
        uint32 buffID;
    };
}