/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_SCRIPTMGR_H
#define SC_SCRIPTMGR_H

#include "Common.h"
#include "DBCStructure.h"

class Player;
class Creature;
class CreatureAI;
class InstanceData;
class Quest;
class Item;
class GameObject;
class SpellCastTargets;
class Map;
class Unit;
class WorldObject;
class Aura;
class Object;
class ObjectGuid;

// *********************************************************
// ************** Some defines used globally ***************

// Basic defines
#define VISIBLE_RANGE       (166.0f)                        // MAX visible range (size of grid)
#define DEFAULT_TEXT        "<ScriptDev2 Text Entry Missing!>"

/* Escort Factions
 * TODO: find better namings and definitions.
 * N=Neutral, A=Alliance, H=Horde.
 * NEUTRAL or FRIEND = Hostility to player surroundings (not a good definition)
 * ACTIVE or PASSIVE = Hostility to environment surroundings.
 */
enum EscortFaction
{
    FACTION_ESCORT_A_NEUTRAL_PASSIVE    = 10,
    FACTION_ESCORT_H_NEUTRAL_PASSIVE    = 33,
    FACTION_ESCORT_N_NEUTRAL_PASSIVE    = 113,

    FACTION_ESCORT_A_NEUTRAL_ACTIVE     = 231,
    FACTION_ESCORT_H_NEUTRAL_ACTIVE     = 232,
    FACTION_ESCORT_N_NEUTRAL_ACTIVE     = 250,

    FACTION_ESCORT_N_FRIEND_PASSIVE     = 290,
    FACTION_ESCORT_N_FRIEND_ACTIVE      = 495,

    FACTION_ESCORT_A_PASSIVE            = 774,
    FACTION_ESCORT_H_PASSIVE            = 775,
};

// *********************************************************
// ************* Some structures used globally *************

struct Script
{
    Script() :
        pGossipHello(nullptr), pGossipHelloGO(nullptr), pGossipSelect(nullptr), pGossipSelectGO(nullptr),
        pGossipSelectWithCode(nullptr), pGossipSelectGOWithCode(nullptr),
        pDialogStatusNPC(nullptr), pDialogStatusGO(nullptr),
        pQuestAcceptNPC(nullptr), pQuestAcceptGO(nullptr), pQuestAcceptItem(nullptr),
        pQuestRewardedNPC(nullptr), pQuestRewardedGO(nullptr),
        pGOUse(nullptr), pItemUse(nullptr), pAreaTrigger(nullptr), pProcessEventId(nullptr),
        pEffectDummyNPC(nullptr), pEffectDummyGO(nullptr), pEffectDummyItem(nullptr), pEffectScriptEffectNPC(nullptr),
        pEffectAuraDummy(nullptr), GetAI(nullptr), GetInstanceData(nullptr)
    {}

    std::string Name;

    bool (*pGossipHello)(Player*, Creature*);
    bool (*pGossipHelloGO)(Player*, GameObject*);
    bool (*pGossipSelect)(Player*, Creature*, uint32, uint32);
    bool (*pGossipSelectGO)(Player*, GameObject*, uint32, uint32);
    bool (*pGossipSelectWithCode)(Player*, Creature*, uint32, uint32, const char*);
    bool (*pGossipSelectGOWithCode)(Player*, GameObject*, uint32, uint32, const char*);
    uint32(*pDialogStatusNPC)(Player*, Creature*);
    uint32(*pDialogStatusGO)(Player*, GameObject*);
    bool (*pQuestAcceptNPC)(Player*, Creature*, Quest const*);
    bool (*pQuestAcceptGO)(Player*, GameObject*, Quest const*);
    bool (*pQuestAcceptItem)(Player*, Item*, Quest const*);
    bool (*pQuestRewardedNPC)(Player*, Creature*, Quest const*);
    bool (*pQuestRewardedGO)(Player*, GameObject*, Quest const*);
    bool (*pGOUse)(Player*, GameObject*);
    bool (*pItemUse)(Player*, Item*, SpellCastTargets const&);
    bool (*pAreaTrigger)(Player*, AreaTriggerEntry const*);
    bool (*pProcessEventId)(uint32, Object*, Object*, bool);
    bool (*pEffectDummyNPC)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
    bool (*pEffectDummyGO)(Unit*, uint32, SpellEffectIndex, GameObject*, ObjectGuid);
    bool (*pEffectDummyItem)(Unit*, uint32, SpellEffectIndex, Item*, ObjectGuid);
    bool (*pEffectScriptEffectNPC)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
    bool (*pEffectAuraDummy)(const Aura*, bool);

    CreatureAI* (*GetAI)(Creature*);
    InstanceData* (*GetInstanceData)(Map*);

    void RegisterSelf(bool bReportError = true);
};

// *********************************************************
// ************* Some functions used globally **************

// Generic scripting text function
void DoScriptText(int32 iTextEntry, WorldObject* pSource, Unit* pTarget = nullptr);
void DoOrSimulateScriptTextForMap(int32 iTextEntry, uint32 uiCreatureEntry, Map* pMap, Creature* pCreatureSource = nullptr, Unit* pTarget = nullptr);

// *********************************************************
// **************** Internal hook mechanics ****************

#if COMPILER == COMPILER_GNU
#define FUNC_PTR(name,callconvention,returntype,parameters)    typedef returntype(*name)parameters __attribute__ ((callconvention));
#else
#define FUNC_PTR(name, callconvention, returntype, parameters)    typedef returntype(callconvention *name)parameters;
#endif

#endif
