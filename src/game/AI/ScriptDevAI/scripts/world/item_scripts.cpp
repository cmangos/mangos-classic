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
SDName: Item_Scripts
SD%Complete: 100
SDComment: quest 4743
SDCategory: Items
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Spells/Spell.h"
#include "Spells/Scripts/SpellScript.h"
/* ContentData
EndContentData */

/*#####
# item_orb_of_draconic_energy
#####*/

enum
{
    SPELL_DOMINION_SOUL     = 16053,
    NPC_EMBERSTRIFE         = 10321
};

bool ItemUse_item_orb_of_draconic_energy(Player* pPlayer, Item* pItem, const SpellCastTargets& /*pTargets*/)
{
    Creature* pEmberstrife = GetClosestCreatureWithEntry(pPlayer, NPC_EMBERSTRIFE, 20.0f);
    if (!pEmberstrife)
        return false;

    // If Emberstrife already mind controled or above 10% HP: force spell cast failure
    if (pEmberstrife->HasAura(SPELL_DOMINION_SOUL) || pEmberstrife->GetHealthPercent() > 10.0f)
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DOMINION_SOUL))
            Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_TARGET_AURASTATE);

        return true;
    }

    return false;
}

enum
{
    SPELL_ASHBRINGER_EFFECT_001 = 28442,
};

struct AshbringerItemAura : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const
    {
        if (apply)
        {
            Unit* target = aura->GetTarget();
            int32 basepoints = ReputationRank(REP_FRIENDLY);
            target->CastCustomSpell(nullptr, SPELL_ASHBRINGER_EFFECT_001, &basepoints, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
        else
            aura->GetTarget()->RemoveAurasDueToSpell(SPELL_ASHBRINGER_EFFECT_001);
    }
};

enum
{
    SPELL_GDR_PERIODIC_DAMAGE = 13493,
    SPELL_GDR_DAMAGE_HIT      = 13279,
};

struct GDRChannel : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_1)
            spell->GetCaster()->CastSpell(nullptr, SPELL_GDR_PERIODIC_DAMAGE, TRIGGERED_OLD_TRIGGERED);
    }
};

struct GDRPeriodicDamage : public AuraScript
{
    int32 OnAuraValueCalculate(AuraCalcData& /*data*/, int32 /*value*/) const override
    {
        return urand(100, 500);
    }

    void OnApply(Aura* aura, bool apply) const override
    {
        if (!apply && aura->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            int32 dmg = (int32)aura->GetScriptValue();
            aura->GetTarget()->CastCustomSpell(aura->GetTarget()->GetTarget(), SPELL_GDR_DAMAGE_HIT, &dmg, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

struct BanishExile : public SpellScript
{
    SpellCastResult OnCheckCast(Spell* spell, bool /*strict*/) const override
    {
        uint32 entry = 0;
        switch (spell->m_spellInfo->Id)
        {
            case 4130: entry = 2760; break; // Burning Exile
            case 4131: entry = 2761; break; // Cresting Exile
            case 4132: entry = 2762; break; // Thundering Exile
        }
        if (ObjectGuid target = spell->m_targets.getUnitTargetGuid()) // can be cast only on this target
            if (target.GetEntry() != entry)
                return SPELL_FAILED_BAD_TARGETS;

        return SPELL_CAST_OK;
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (!target)
            return;

        DoScriptText(-1010004, target, spell->GetCaster());
        target->CastSpell(nullptr, 3617, TRIGGERED_OLD_TRIGGERED);
    }
};

struct OrbOfDeception : public AuraScript
{
    uint32 GetAuraScriptCustomizationValue(Aura* aura) const override
    {
        uint32 orbModel = aura->GetTarget()->GetNativeDisplayId();
        uint32 resultingModel = 0; // keeping this for future research
        uint32 resultingTemplate = 0;
        switch (orbModel)
        {
            // Troll Female
            case 1479: resultingModel = 10134; resultingTemplate = 10792; break;
            // Troll Male
            case 1478: resultingModel = 10135; resultingTemplate = 10791; break;
            // Tauren Male
            case 59:   resultingModel = 10136; resultingTemplate = 10793; break;
            // Human Male
            case 49:   resultingModel = 10137; resultingTemplate = 10783; break;
            // Human Female
            case 50:   resultingModel = 10138; resultingTemplate = 10784; break;
            // Orc Male
            case 51:   resultingModel = 10139; resultingTemplate = 10795; break;
            // Orc Female
            case 52:   resultingModel = 10140; resultingTemplate = 10796; break;
            // Dwarf Male
            case 53:   resultingModel = 10141; resultingTemplate = 10787; break;
            // Dwarf Female
            case 54:   resultingModel = 10142; resultingTemplate = 10788; break;
            // NightElf Male
            case 55:   resultingModel = 10143; resultingTemplate = 10789; break;
            // NightElf Female
            case 56:   resultingModel = 10144; resultingTemplate = 10790; break;
            // Undead Female
            case 58:   resultingModel = 10145; resultingTemplate = 10798; break;
            // Undead Male
            case 57:   resultingModel = 10146; resultingTemplate = 10797; break;
            // Tauren Female
            case 60:   resultingModel = 10147; resultingTemplate = 10794; break;
            // Gnome Male
            case 1563: resultingModel = 10148; resultingTemplate = 10785; break;
            // Gnome Female
            case 1564: resultingModel = 10149; resultingTemplate = 10786; break;
            default: break;
        }
        return resultingModel;
    }
};

enum
{
    SPELL_MALFUNCTION_EXPLOSION = 13261,
    SPELL_SUMMON_GOBLIN_BOMB    = 13258,
    SPELL_PET_BOMB_PASSIVE      = 13260, // triggers 13259 Explosion on successful melee hit
};

// 13258 - Summon Goblin Bomb
struct SummonGoblinBomb : public SpellScript
{
    void OnSummon(Spell* spell, Creature* summon) const override
    {
        if (Player* player = dynamic_cast<Player*>(spell->GetCaster()))
            summon->SelectLevel(uint16(player->GetSkillValue(SKILL_ENGINEERING) / 5));
        summon->AI()->SetReactState(REACT_AGGRESSIVE);
        summon->CastSpell(summon, SPELL_PET_BOMB_PASSIVE, TRIGGERED_OLD_TRIGGERED);
    }
};

// 23134 - Goblin Bomb
struct GoblinBomb : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* target = spell->GetUnitTarget();
        Unit* caster = spell->GetCaster();
        if (!target || !caster)
            return;
        uint32 roll = urand(0, 99);
        if (roll < 10)
            target->CastSpell(caster, SPELL_MALFUNCTION_EXPLOSION, TRIGGERED_OLD_TRIGGERED);
        else
            caster->CastSpell(caster, SPELL_SUMMON_GOBLIN_BOMB, TRIGGERED_OLD_TRIGGERED);
    }
};

// 11403 - Dream Vision
struct DreamVision : public SpellScript
{
    void OnSummon(Spell* /*spell*/, Creature* summon) const override
    {
        if (summon->GetEntry() != 7863)
            return;

        summon->SetHover(true);
        summon->SetWaterWalk(true);
        summon->SetFeatherFall(true);
    }

    void OnRadiusCalculate(Spell* /*spell*/, SpellEffectIndex effIdx, bool /*targetB*/, float& radius) const override
    {
        if (effIdx != EFFECT_INDEX_0)
            return;
        radius = 2.f;
    }
};

// 26656 - Summon Black Qiraji Battle Tank
struct SummonBlackQirajiBattleTank : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* unitTarget = spell->GetUnitTarget();
        if (!unitTarget)
            return;

        // Prevent stacking of mounts
        unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        // Two separate mounts depending on area id (allows use both in and out of specific instance)
        switch (unitTarget->GetAreaId())
        {
            [[unlikely]] case 3428: unitTarget->CastSpell(nullptr, 25863, TRIGGERED_NONE); break;
            default: unitTarget->CastSpell(nullptr, 26655, TRIGGERED_NONE);
        }
    }
};

// 8342 - Defibrillate
// 22999 - Defibrillate
struct GoblinJumperCables : public SpellScript
{
    void OnInit(Spell* spell) const override
    {
        uint32 failChance = 0;
        switch (spell->m_spellInfo->Id)
        {
            case 8342: failChance = 67; break;
            case 22999: failChance = 50; break;
        }
        spell->SetEffectChance(failChance, EFFECT_INDEX_0);
        spell->SetScriptValue(1);
    }

    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        spell->SetScriptValue(0);
    }

    void OnSuccessfulFinish(Spell* spell) const override
    { 
        if (spell->GetScriptValue() == 1)
        {
            uint32 failSpellId = 0;
            switch (spell->m_spellInfo->Id)
            {
                case 8342: failSpellId = 8338; break;
                case 22999: failSpellId = 23055; break;
            }

            if (failSpellId)
                spell->GetCaster()->CastSpell(nullptr, failSpellId, TRIGGERED_OLD_TRIGGERED, spell->GetCastItem());
        }
    }
};

// 23441 - Gadgetzan Transporter
struct GadgetzanTransporter : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        // Wrong destination already rolled for, only handle minor malfunction on sucess
        spell->GetCaster()->CastSpell(nullptr, 23450, TRIGGERED_OLD_TRIGGERED); // Transporter Arrival
    }
};

// 23442 - Everlook Transporter
struct EverlookTransporter : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        // Roll for major malfunction (1/6); 23450 = success | 23449 = malfunction (being set afire)
        spell->GetCaster()->CastSpell(nullptr, (urand(0, 5) ? 23450 : 23449), TRIGGERED_OLD_TRIGGERED);
    }
};

// 36941 - Toshley's Station Transporter
struct ToshleysStationTransporter : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        Unit* caster = spell->GetCaster();
        if (roll_chance_i(50)) // 50% success
        {
            int32 rand_eff = urand(1, 7);
            switch (rand_eff)
            {
                case 1:
                    // soul split - evil
                    caster->CastSpell(nullptr, 36900, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 2:
                    // soul split - good
                    caster->CastSpell(nullptr, 36901, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 3:
                    // Increase the size
                    caster->CastSpell(nullptr, 36895, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 4:
                    // Decrease the size
                    caster->CastSpell(nullptr, 36893, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 5:
                    // Transform
                    {
                        if (caster->IsPlayer() && static_cast<Player*>(caster)->GetTeam() == ALLIANCE)
                            caster->CastSpell(nullptr, 36897, TRIGGERED_OLD_TRIGGERED);
                        else
                            caster->CastSpell(nullptr, 36899, TRIGGERED_OLD_TRIGGERED);
                        break;
                    }
                case 6:
                    // chicken
                    caster->CastSpell(nullptr, 36940, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 7:
                    // evil twin
                    caster->CastSpell(nullptr, 23445, TRIGGERED_OLD_TRIGGERED);
                    break;
            }
        }
    }
};

// 36890 - Area52 Transporter
struct Area52Transporter : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        Unit* caster = spell->GetCaster();
        if (roll_chance_i(50)) // 50% success
        {
            int32 rand_eff = urand(1, 4);
            switch (rand_eff)
            {
                case 1:
                    // soul split - evil
                    caster->CastSpell(nullptr, 36900, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 2:
                    // soul split - good
                    caster->CastSpell(nullptr, 36901, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 3:
                    // Increase the size
                    caster->CastSpell(nullptr, 36895, TRIGGERED_OLD_TRIGGERED);
                    break;
                case 4:
                    // Transform
                    {
                        if (caster->IsPlayer() && static_cast<Player*>(caster)->GetTeam() == ALLIANCE)
                            caster->CastSpell(nullptr, 36897, TRIGGERED_OLD_TRIGGERED);
                        else
                            caster->CastSpell(nullptr, 36899, TRIGGERED_OLD_TRIGGERED);
                        break;
                    }
            }
        }
    }
};

void AddSC_item_scripts()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "item_orb_of_draconic_energy";
    pNewScript->pItemUse = &ItemUse_item_orb_of_draconic_energy;
    pNewScript->RegisterSelf();

    RegisterSpellScript<AshbringerItemAura>("spell_ashbringer_item");

    RegisterSpellScript<GDRChannel>("spell_gdr_channel");
    RegisterSpellScript<GDRPeriodicDamage>("spell_gdr_periodic");
    RegisterSpellScript<BanishExile>("spell_banish_exile");
    RegisterSpellScript<OrbOfDeception>("spell_orb_of_deception");
    RegisterSpellScript<SummonGoblinBomb>("spell_summon_goblin_bomb");
    RegisterSpellScript<GoblinBomb>("spell_goblin_bomb");
    RegisterSpellScript<DreamVision>("spell_dream_vision");
    RegisterSpellScript<SummonBlackQirajiBattleTank>("spell_summon_black_qiraji_battle_tank");
    RegisterSpellScript<GoblinJumperCables>("spell_goblin_jumper_cables");
    RegisterSpellScript<GadgetzanTransporter>("spell_gadgetzan_transporter");
    RegisterSpellScript<EverlookTransporter>("spell_everlook_transporter");
    RegisterSpellScript<ToshleysStationTransporter>("spell_toshleys_station_transporter");
    RegisterSpellScript<Area52Transporter>("spell_area52_transporter");
}
