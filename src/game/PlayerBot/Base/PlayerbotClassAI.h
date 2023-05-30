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

#ifndef _PLAYERBOTCLASSAI_H
#define _PLAYERBOTCLASSAI_H

#include "Common.h"
#include "Server/WorldPacket.h"
#include "PlayerbotAI.h"
#include "../../Entities/Player.h"
#include "../../Entities/Unit.h"
#include "../../Globals/ObjectMgr.h"
#include "../../Globals/ObjectAccessor.h"
#include "../../Globals/SharedDefines.h"
#include "../../Spells/SpellMgr.h"
#include "../../Spells/SpellAuras.h"
#include "../../World/World.h"

class Player;
class PlayerbotAI;

enum JOB_TYPE
{
    JOB_MAIN_TANK   = 0x01,     // for main tank that will need specific heal focus and priority over other party members or even regular tanks
    JOB_MAIN_HEAL   = 0x02,     // for healers that will focus on main tank
    JOB_HEAL        = 0x04,
    JOB_TANK        = 0x08,
    JOB_MASTER      = 0x10,     // Not a fan of this distinction but user (or rather, admin) choice
    JOB_TANK_MASTER = 0x19,
    JOB_DPS         = 0x20,
    JOB_ALL_NO_MT   = 0x3E,     // all of the above except Main Tank
    JOB_ALL         = 0x3F,     // all of the above
    JOB_MANAONLY    = 0x40      // for buff checking (NOTE: this means any with powertype mana AND druids (who may be shifted but still have mana)
};

struct heal_priority
{
    Player* p;
    uint8 hp;
    JOB_TYPE type;
    heal_priority(Player* pin, uint8 hpin, JOB_TYPE t) : p(pin), hp(hpin), type(t) {}
    // overriding the operator like this is not recommended for general use - however we won't use this struct for anything else
    bool operator<(const heal_priority& a) const { return type < a.type; }
};

class PlayerbotClassAI
{
    public:
        PlayerbotClassAI(Player& master, Player& bot, PlayerbotAI& ai);
        virtual ~PlayerbotClassAI();

        // all combat actions go here
        virtual CombatManeuverReturns DoFirstCombatManeuver(Unit*);
        virtual CombatManeuverReturns DoNextCombatManeuver(Unit*);
        virtual bool CanPull() { return false; }
        virtual bool Pull() { return false; }
        virtual uint32 Neutralize(uint8 creatureType) { return 0; }

        // all non combat actions go here, ex buffs, heals, rezzes
        virtual void DoNonCombatActions();
        bool EatDrinkBandage(bool bMana = true, unsigned char foodPercent = 50, unsigned char drinkPercent = 50, unsigned char bandagePercent = 70);

        // Utilities
        bool CastHoTOnTank();
        JOB_TYPE GetBotJob(Player* target);
        JOB_TYPE GetTargetJob(Player* target);
        time_t GetWaitUntil() { return m_WaitUntil; }
        void SetWait(uint8 t) { m_WaitUntil = m_ai.CurrentTime() + t; }
        void ClearWait() { m_WaitUntil = 0; }
        //void SetWaitUntil(time_t t) { m_WaitUntil = t; }

    protected:
        virtual CombatManeuverReturns DoFirstCombatManeuverPVE(Unit*);
        virtual CombatManeuverReturns DoNextCombatManeuverPVE(Unit*);
        virtual CombatManeuverReturns DoFirstCombatManeuverPVP(Unit*);
        virtual CombatManeuverReturns DoNextCombatManeuverPVP(Unit*);

        CombatManeuverReturns CastSpellNoRanged(uint32 nextAction, Unit* pTarget);
        CombatManeuverReturns CastSpellWand(uint32 nextAction, Unit* pTarget, uint32 SHOOT);
        virtual CombatManeuverReturns HealPlayer(Player* target);
        virtual CombatManeuverReturns ResurrectPlayer(Player* target);
        virtual CombatManeuverReturns DispelPlayer(Player* target);
        CombatManeuverReturns Buff(bool (*BuffHelper)(PlayerbotAI*, uint32, Unit*), uint32 spellId, uint32 type = JOB_ALL, bool mustBeOOC = true);
        bool FindTargetAndHeal();
        bool NeedGroupBuff(uint32 groupBuffSpellId, uint32 singleBuffSpellId);
        Player* GetHealTarget(JOB_TYPE type = JOB_ALL, bool onlyPickFromSameGroup = false);
        Player* GetDispelTarget(DispelType dispelType, JOB_TYPE type = JOB_ALL, bool bMustBeOOC = false);
        Player* GetResurrectionTarget(JOB_TYPE type = JOB_ALL, bool bMustBeOOC = true);

        bool FleeFromAoEIfCan(uint32 spellId, Unit* pTarget);
        bool FleeFromTrapGOIfCan(uint32 goEntry, Unit* pTarget);
        bool FleeFromNpcWithAuraIfCan(uint32 NpcEntry, uint32 spellId, Unit* pTarget);
        bool FleeFromPointIfCan(uint32 radius, Unit* pTarget, float x0, float y0, float z0, float forcedAngle = 0.0f);

        // These values are used in GetHealTarget and can be overridden per class (to accomodate healing spell health checks)
        uint8 m_MinHealthPercentTank;
        uint8 m_MinHealthPercentHealer;
        uint8 m_MinHealthPercentDPS;
        uint8 m_MinHealthPercentMaster;

        time_t m_WaitUntil;

        Player& m_master;
        Player& m_bot;
        PlayerbotAI& m_ai;

        // first aid
        uint32 RECENTLY_BANDAGED;
};

#endif
