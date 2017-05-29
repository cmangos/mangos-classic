#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;
class PlayerbotClassAI;

typedef std::unordered_map<ObjectGuid, Player*> PlayerBotMap;

class MANGOS_DLL_SPEC PlayerbotMgr
{
    // static functions, available without a PlayerbotMgr instance
public:
    static void SetInitialWorldSettings();

public:
    PlayerbotMgr(Player * const master);
    virtual ~PlayerbotMgr();

    // This is called from Unit.cpp and is called every second (I think)
    void UpdateAI(const uint32 p_time);

    // This is called whenever the master sends a packet to the server.
    // These packets can be viewed, but not edited.
    // It allows bot creators to craft AI in response to a master's actions.
    // For a list of opcodes that can be caught see Opcodes.cpp (CMSG_* opcodes only)
    // Notice: that this is static which means it is called once for all bots of the master.
    void HandleMasterIncomingPacket(const WorldPacket& packet);
    void HandleMasterOutgoingPacket(const WorldPacket& packet);

    void LoginPlayerBot(ObjectGuid guid);
    void LogoutPlayerBot(ObjectGuid guid);
    Player* GetPlayerBot (ObjectGuid guid) const;
    Player* GetMaster() const { return m_master; };
    PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return m_playerBots.begin(); }
    PlayerBotMap::const_iterator GetPlayerBotsEnd()   const { return m_playerBots.end();   }

    void LogoutAllBots();
    void RemoveAllBotsFromGroup();
    void OnBotLogin(Player * const bot);
    void Stay();

public:
    // config variables
    uint32 m_confRestrictBotLevel;
    uint32 m_confDisableBotsInRealm;
    uint32 m_confMaxNumBots;
    bool m_confDisableBots;
    bool m_confDebugWhisper;
    float m_confFollowDistance[2];
    bool m_confCollectCombat;
    bool m_confCollectQuest;
    bool m_confCollectProfession;
    bool m_confCollectLoot;
    bool m_confCollectSkin;
    bool m_confCollectObjects;

private:
    Player* const m_master;
    PlayerBotMap m_playerBots;
};

#endif
