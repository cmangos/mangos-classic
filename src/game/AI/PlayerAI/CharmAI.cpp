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

#include "PlayerAI.h"

enum PriestSpells
{
    SPELL_MIND_BLAST = 8092,
};

enum PriestPriorities
{
    PRIORITY_MIND_BLAST,
    PRIEST_SPELL_MAX,
};

struct PriestAI : public PlayerAI
{
    PriestAI(Player* player) : PlayerAI(player)
    {
        if (uint32 mindBlast = LookupHighestLearnedRank(SPELL_MIND_BLAST))
            AddPlayerSpellAction(PRIORITY_MIND_BLAST, mindBlast);
    }
};

enum MageSpells
{
    SPELL_FIREBALL = 133,
};

enum MagePriorities
{
    PRIORITY_FIREBALL,
    MAGE_SPELL_MAX,
};

struct MageAI : public PlayerAI
{
    MageAI(Player* player) : PlayerAI(player)
    {
        if (uint32 fireball = LookupHighestLearnedRank(SPELL_FIREBALL))
            AddPlayerSpellAction(PRIORITY_FIREBALL, fireball);
    }
};

enum WarlockSpells
{
    SPELL_SHADOW_BOLT = 686,
};

enum WarlockPriorities
{
    PRIORITY_SHADOW_BOLT,
    WARLOCK_SPELL_MAX,
};

struct WarlockAI : public PlayerAI
{
    WarlockAI(Player* player) : PlayerAI(player)
    {
        if (uint32 shadowBolt = LookupHighestLearnedRank(SPELL_SHADOW_BOLT))
            AddPlayerSpellAction(PRIORITY_SHADOW_BOLT, shadowBolt);
    }
};

enum RogueSpells
{
    SPELL_SINISTER_STRIKE = 1752,
};

enum RoguePriorities
{
    PRIORITY_SINISTER_STRIKE,
    ROGUE_SPELL_MAX,
};

struct RogueAI : public PlayerAI
{
    RogueAI(Player* player) : PlayerAI(player)
    {
        if (uint32 sinisterStrike = LookupHighestLearnedRank(SPELL_SINISTER_STRIKE))
            AddPlayerSpellAction(PRIORITY_SINISTER_STRIKE, sinisterStrike);
    }
};

enum DruidSpells
{
    SPELL_MOONFIRE = 8921,
};

enum DruidPriorities
{
    PRIORITY_MOONFIRE,
    DRUID_SPELL_MAX,
};

struct DruidAI : public PlayerAI
{
    DruidAI(Player* player) : PlayerAI(player)
    {
        if (uint32 moonfire = LookupHighestLearnedRank(SPELL_MOONFIRE))
            AddPlayerSpellAction(PRIORITY_MOONFIRE, moonfire);
    }
};

enum ShamanSpells
{
    SPELL_FLAME_SHOCK = 8050,
};

enum ShamanPriorities
{
    PRIORITY_FLAME_SHOCK,
    SHAMAN_SPELL_MAX,
};

struct ShamanAI : public PlayerAI
{
    ShamanAI(Player* player) : PlayerAI(player)
    {
        if (uint32 flameShock = LookupHighestLearnedRank(SPELL_FLAME_SHOCK))
            AddPlayerSpellAction(PRIORITY_FLAME_SHOCK, flameShock);
    }
};

enum HunterSpells
{
    SPELL_RAPTOR_STRIKE = 2973,
};

enum HunterPriorities
{
    PRIORITY_RAPTOR_STRIKE,
    HUNTER_SPELL_MAX,
};

struct HunterAI : public PlayerAI
{
    HunterAI(Player* player) : PlayerAI(player)
    {
        if (uint32 raptorStrike = LookupHighestLearnedRank(SPELL_RAPTOR_STRIKE))
            AddPlayerSpellAction(PRIORITY_RAPTOR_STRIKE, raptorStrike);
    }
};

enum WarriorSpells
{
    SPELL_HEROIC_STRIKE = 78,
};

enum WarriorPriorities
{
    PRIORITY_HEROIC_STRIKE = 0,
    WARRIOR_SPELL_MAX,
};

struct WarriorAI : public PlayerAI
{
    WarriorAI(Player* player) : PlayerAI(player)
    {
        if (uint32 heroicStrike = LookupHighestLearnedRank(SPELL_HEROIC_STRIKE))
            AddPlayerSpellAction(PRIORITY_HEROIC_STRIKE, heroicStrike);
    }
};

enum PaladinSpells
{

};

enum PaladinPriorities
{
    PALADIN_SPELL_MAX,
};

struct PaladinAI : public PlayerAI
{
    PaladinAI(Player* player) : PlayerAI(player)
    {

    }
};

UnitAI* GetClassAI(Classes playerClass, Player* player)
{
    switch (playerClass)
    {
        case CLASS_WARRIOR: return new WarriorAI(player);
        case CLASS_PALADIN: return new PaladinAI(player);
        case CLASS_HUNTER:  return new HunterAI(player);
        case CLASS_ROGUE:   return new RogueAI(player);
        case CLASS_PRIEST:  return new PriestAI(player);
        case CLASS_SHAMAN:  return new ShamanAI(player);
        case CLASS_MAGE:    return new MageAI(player);
        case CLASS_WARLOCK: return new WarlockAI(player);
        case CLASS_DRUID:   return new DruidAI(player);
        default: return new PlayerAI(player);
    }
}