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
    SPELL_PSYCHIC_SCREAM = 8122,
    SPELL_SHADOW_WORD_PAIN = 589,
    SPELL_MIND_BLAST = 8092,
};

struct PriestAI : public PlayerAI
{
    PriestAI(Player* player) : PlayerAI(player)
    {
        if (uint32 mindBlast = m_player->LookupHighestLearnedRank(SPELL_MIND_BLAST))
            AddPlayerSpellAction(mindBlast);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_SHADOW_WORD_PAIN))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_PSYCHIC_SCREAM))
            AddPlayerSpellAction(spell);
    }
};

enum MageSpells
{
    SPELL_BLIZZARD = 10,
    SPELL_FROSTBOLT = 116,
    SPELL_FIREBALL = 133,
};

struct MageAI : public PlayerAI
{
    MageAI(Player* player) : PlayerAI(player)
    {
        if (uint32 fireball = m_player->LookupHighestLearnedRank(SPELL_FIREBALL))
            AddPlayerSpellAction(fireball);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_BLIZZARD))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_FROSTBOLT))
            AddPlayerSpellAction(spell);
    }
};

enum WarlockSpells
{
    SPELL_RAIN_OF_FIRE = 5740,
    SPELL_FEAR = 5782,
    SPELL_SHADOW_BOLT = 686,
};

struct WarlockAI : public PlayerAI
{
    WarlockAI(Player* player) : PlayerAI(player)
    {
        if (uint32 shadowBolt = m_player->LookupHighestLearnedRank(SPELL_SHADOW_BOLT))
            AddPlayerSpellAction(shadowBolt);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_RAIN_OF_FIRE))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_FEAR))
            AddPlayerSpellAction(spell);
    }
};

enum RogueSpells
{
    SPELL_BLIND = 2094,
    SPELL_EVISCERATE = 2098,
    SPELL_SINISTER_STRIKE = 1752,
};

struct RogueAI : public PlayerAI
{
    RogueAI(Player* player) : PlayerAI(player)
    {
        if (uint32 sinisterStrike = m_player->LookupHighestLearnedRank(SPELL_SINISTER_STRIKE))
            AddPlayerSpellAction(sinisterStrike);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_BLIND))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_EVISCERATE))
            AddPlayerSpellAction(spell);
    }
};

enum DruidSpells
{
    SPELL_WRATH = 5176,
    SPELL_HURRICANE = 16914,
    SPELL_MOONFIRE = 8921,
};

struct DruidAI : public PlayerAI
{
    DruidAI(Player* player) : PlayerAI(player)
    {
        if (uint32 moonfire = m_player->LookupHighestLearnedRank(SPELL_MOONFIRE))
            AddPlayerSpellAction(moonfire);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_WRATH))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_HURRICANE))
            AddPlayerSpellAction(spell);
    }
};

enum ShamanSpells
{
    SPELL_HEROISM = 32182,
    SPELL_BLOODLUST = 2825,
    SPELL_LIGHTNING_BOLT = 403,
    SPELL_FLAME_SHOCK = 8050,
};

struct ShamanAI : public PlayerAI
{
    ShamanAI(Player* player) : PlayerAI(player)
    {
        if (uint32 flameShock = m_player->LookupHighestLearnedRank(SPELL_FLAME_SHOCK))
            AddPlayerSpellAction(flameShock);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_HEROISM))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_BLOODLUST))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_LIGHTNING_BOLT))
            AddPlayerSpellAction(spell);
    }
};

enum HunterSpells
{
    SPELL_VOLLEY = 1510,
    SPELL_EXPLOSIVE_TRAP = 13813,
    SPELL_RAPTOR_STRIKE = 2973,
};

struct HunterAI : public PlayerAI
{
    HunterAI(Player* player) : PlayerAI(player)
    {
        if (uint32 raptorStrike = m_player->LookupHighestLearnedRank(SPELL_RAPTOR_STRIKE))
            AddPlayerSpellAction(raptorStrike);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_VOLLEY))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_EXPLOSIVE_TRAP))
            AddPlayerSpellAction(spell);
    }
};

enum WarriorSpells
{
    SPELL_INTIMIDATING_SHOUT = 5246,
    SPELL_HAMSTRING = 1715,
    SPELL_HEROIC_STRIKE = 78,
};

struct WarriorAI : public PlayerAI
{
    WarriorAI(Player* player) : PlayerAI(player)
    {
        if (uint32 heroicStrike = m_player->LookupHighestLearnedRank(SPELL_HEROIC_STRIKE))
            AddPlayerSpellAction(heroicStrike);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_INTIMIDATING_SHOUT))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_HAMSTRING))
            AddPlayerSpellAction(spell);
    }
};

enum PaladinSpells
{
    SPELL_FLASH_OF_LIGHT = 19750,
    SPELL_HAMMER_OF_JUSTICE = 853,
    SPELL_CONSECRATION = 26573,
};

struct PaladinAI : public PlayerAI
{
    PaladinAI(Player* player) : PlayerAI(player)
    {
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_FLASH_OF_LIGHT))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_HAMMER_OF_JUSTICE))
            AddPlayerSpellAction(spell);
        if (uint32 spell = m_player->LookupHighestLearnedRank(SPELL_CONSECRATION))
            AddPlayerSpellAction(spell);
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