/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

class PlayerbotMgr
{
        // static functions, available without a PlayerbotMgr instance
    public:
        static void SetInitialWorldSettings();

    public:
        PlayerbotMgr(Player* const master);
        virtual ~PlayerbotMgr();

        // remove marked bot
        // should be called from worldsession::Update only to avoid possible problem with invalid session or player pointer
        void RemoveBots();

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
        void LogoutPlayerBot(ObjectGuid guid);          // mark bot to be removed on next update
        Player* GetPlayerBot(ObjectGuid guid) const;
        Player* GetMaster() const { return m_master; };
        PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return m_playerBots.begin(); }
        PlayerBotMap::const_iterator GetPlayerBotsEnd()   const { return m_playerBots.end();   }

        void LogoutAllBots(bool fullRemove = false);                           // mark all bots to be removed on next update
        void RemoveAllBotsFromGroup();
        void OnBotLogin(Player* const bot);
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
        uint32 m_confCollectDistance;
        uint32 m_confCollectDistanceMax;

    private:
        Player* const m_master;
        PlayerBotMap m_playerBots;
        GuidSet m_botToRemove;
};

#endif
