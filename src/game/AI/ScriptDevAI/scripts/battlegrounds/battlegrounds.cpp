/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

/* ScriptData
SDName: Battleground
SD%Complete: 100
SDComment: Spirit guides in battlegrounds will revive all players every 30 sec
SDCategory: Battlegrounds
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Spells/Scripts/SpellScript.h"
#include "OutdoorPvP/OutdoorPvP.h"

// **** Script Info ****
// Spiritguides in battlegrounds resurrecting many players at once
// every 30 seconds - through a channeled spell, which gets autocasted
// the whole time
// if spiritguide despawns all players around him will get teleported
// to the next spiritguide
// here i'm not sure, if a dummyspell exist for it

// **** Quick Info ****
// battleground spiritguides - this script handles gossipHello
// and JustDied also it let autocast the channel-spell

enum
{
    SPELL_SPIRIT_HEAL_CHANNEL       = 22011,                // Spirit Heal Channel

    SPELL_SPIRIT_HEAL               = 22012,                // Spirit Heal

    SPELL_WAITING_TO_RESURRECT      = 2584                  // players who cancel this aura don't want a resurrection
};

struct npc_spirit_guideAI : public ScriptedAI
{
    npc_spirit_guideAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pCreature->SetActiveObjectState(true);
        Reset();
    }

    void Reset() override {}

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        // auto cast the whole time this spell
        if (!m_creature->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            m_creature->CastSpell(m_creature, SPELL_SPIRIT_HEAL_CHANNEL, TRIGGERED_NONE);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            m_creature->InterruptSpell(CURRENT_CHANNELED_SPELL);
            m_creature->CastSpell(nullptr, SPELL_GRAVEYARD_TELEPORT, TRIGGERED_OLD_TRIGGERED);
        }
    }

    void CorpseRemoved(uint32&) override
    {
        // TODO: would be better to cast a dummy spell
        Map* pMap = m_creature->GetMap();

        if (!pMap || !pMap->IsBattleGround())
            return;

        Map::PlayerList const& PlayerList = pMap->GetPlayers();

        for (const auto& itr : PlayerList)
        {
            Player* pPlayer = itr.getSource();
            if (!pPlayer || !pPlayer->IsWithinDistInMap(m_creature, 20.0f) || !pPlayer->HasAura(SPELL_WAITING_TO_RESURRECT))
                continue;

            // repop player again - now this node won't be counted and another node is searched
            pPlayer->RepopAtGraveyard();
        }
    }
};

bool GossipHello_npc_spirit_guide(Player* pPlayer, Creature* /*pCreature*/)
{
    pPlayer->CastSpell(pPlayer, SPELL_WAITING_TO_RESURRECT, TRIGGERED_OLD_TRIGGERED);
    return true;
}

enum
{
    SPELL_OPENING_ANIM = 24390,
};

struct OpeningCapping : public SpellScript
{
    void OnSuccessfulStart(Spell* spell) const
    {
        spell->GetCaster()->CastSpell(nullptr, SPELL_OPENING_ANIM, TRIGGERED_OLD_TRIGGERED);
    }
};

struct FlagAuraBg : public AuraScript, public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
            spell->SetEventTarget(spell->GetAffectiveCasterObject());
    }

    SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const override
    {
        // procs on taken spell - if acquired immune flag, remove it - maybe other conditions too
        if (procData.victim && procData.victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE))
            aura->GetTarget()->RemoveSpellAuraHolder(aura->GetHolder());
        return SPELL_AURA_PROC_OK;
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        Unit* unitTarget = aura->GetTarget();
        if (!unitTarget || !unitTarget->IsPlayer())
            return;

        Player* player = static_cast<Player*>(unitTarget);

        if (apply)
            player->pvpInfo.isPvPFlagCarrier = true;
        else
        {
            player->pvpInfo.isPvPFlagCarrier = false;

            if (BattleGround* bg = player->GetBattleGround())
                bg->HandlePlayerDroppedFlag(player);
            else if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(player->GetCachedZoneId()))
                outdoorPvP->HandleDropFlag(player, aura->GetSpellProto()->Id);
        }
    }
};

struct FlagClickBg : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        switch (spell->m_spellInfo->Id)
        {
            case 23333:                                         // Warsong Flag
            case 23335:                                         // Silverwing Flag
                return spell->GetTrueCaster()->GetMapId() == 489 && spell->GetTrueCaster()->GetMap()->IsBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
            case 34976:                                         // Netherstorm Flag
                return spell->GetTrueCaster()->GetMapId() == 566 && spell->GetTrueCaster()->GetMap()->IsBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        }
        return SPELL_CAST_OK;
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        Unit* target = spell->GetUnitTarget();
        uint32 spellId = 0;
        switch (spell->m_spellInfo->Id)
        {
            case 23383: spellId = 23335; break; // Alliance Flag Pickup
            case 23384: spellId = 23333; break; // Horde Flag Pickup
        }

        // flagstand and flagdrop share spells
        if (spell->GetTrueCaster()->IsGameObject() && static_cast<GameObject*>(spell->GetTrueCaster())->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)
        {
            if (WorldObject* spawner = static_cast<GameObject*>(spell->GetTrueCaster())->GetSpawner())
                if (spawner->IsPlayer() && target->IsPlayer())
                    if (static_cast<Player*>(spawner)->GetTeam() != static_cast<Player*>(target)->GetTeam())
                        return; // return of own flag already handled
        }

        // misusing original caster to pass along original flag GO - if in future conflicts, substitute it for something else
        target->CastSpell(target, spellId, TRIGGERED_IGNORE_GCD | TRIGGERED_HIDE_CAST_IN_COMBAT_LOG | TRIGGERED_IGNORE_CURRENT_CASTED_SPELL, nullptr, nullptr, spell->GetTrueCaster()->GetObjectGuid());
    }
};

/*#####
# spell_battleground_banner_trigger
#
# These are generic spells that handle player click on battleground banners; All spells are triggered by GO type 10
# Contains following spells:
# Arathi Basin: 23932, 23935, 23936, 23937, 23938
# Alterac Valley: 24677
#####*/
struct spell_battleground_banner_trigger : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        // TODO: Fix when go casting is fixed
        WorldObject* obj = spell->GetAffectiveCasterObject();

        if (obj->IsGameObject() && spell->GetUnitTarget()->IsPlayer())
        {
            Player* player = static_cast<Player*>(spell->GetUnitTarget());
            if (BattleGround* bg = player->GetBattleGround())
                bg->HandlePlayerClickedOnFlag(player, static_cast<GameObject*>(obj));
        }
    }
};

struct OutdoorPvpNotifyAI : public GameObjectAI
{
    using GameObjectAI::GameObjectAI;

    void OnUse(Unit* user, SpellEntry const* spellInfo) override
    {
        if (!user->IsPlayer())
            return;

        Player* player = static_cast<Player*>(user);
        if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(player->GetCachedZoneId()))
            outdoorPvP->HandleGameObjectUse(player, m_go);
    }
};

void AddSC_battleground()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_spirit_guide";
    pNewScript->GetAI = &GetNewAIInstance<npc_spirit_guideAI>;
    pNewScript->pGossipHello = &GossipHello_npc_spirit_guide;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_outdoor_pvp_notify";
    pNewScript->GetGameObjectAI = &GetNewAIInstance<OutdoorPvpNotifyAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<OpeningCapping>("spell_opening_capping");
    RegisterSpellScript<FlagAuraBg>("spell_flag_aura_bg");
    RegisterSpellScript<FlagClickBg>("spell_flag_click_bg");
    RegisterSpellScript<spell_battleground_banner_trigger>("spell_battleground_banner_trigger");
}
