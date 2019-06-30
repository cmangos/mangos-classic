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

#ifndef SPELLSCRIPT_H
#define SPELLSCRIPT_H

#include "Spells/Spell.h"
#include <map>
#include <functional>

struct PeriodicTriggerData
{
    Unit* caster; Unit* target; WorldObject* targetObject;
    SpellEntry const* spellInfo;
    int32* basePoints;
    PeriodicTriggerData(Unit* caster, Unit* target, WorldObject* targetObject, SpellEntry const* spellInfo, int32* basePoints) :
        caster(caster), target(target), targetObject(targetObject), spellInfo(spellInfo), basePoints(basePoints) {}
};

struct SpellScript
{
    // called on spell init
    virtual void OnInit(Spell* spell) const {}
    // called on success during Spell::Prepare
    virtual void OnSuccessfulStart(Spell* spell) const {}
    // called on success inside Spell::finish - for channels this only happens if whole channel went through
    virtual void OnSuccessfulFinish(Spell* spell) const {}
    // called at end of Spell::CheckCast - strict is true in Spell::Prepare
    virtual SpellCastResult OnCheckCast(Spell* spell, bool strict) const { return SPELL_CAST_OK; }
    // called before effect execution
    virtual void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const {}
    // called on adding dest target
    virtual void OnDestTarget(Spell* spell) const {}
    // called on Unit Spell::CheckTarget
    virtual bool OnCheckTarget(const Spell* spell, GameObject* target, SpellEffectIndex eff) const { return true; }
    // called on GameObject target checking
    virtual bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex eff) const { return true; }
    // called in Spell::cast on all successful checks and after taking reagents
    virtual void OnCast(Spell* spell) const {}
    // called on target hit before damage deal and proc
    virtual void OnHit(Spell* spell) const {}
    // called on target hit after damage deal and proc
    virtual void OnAfterHit(Spell* spell) const {}
};

struct AuraScript
{
    // called on SpellAuraHolder creation
    virtual void OnHolderInit(SpellAuraHolder* holder) const {}
    // called during any event that calculates aura modifier amount - caster can be nullptr
    virtual int32 OnDamageCalculate(Aura* aura, Unit* caster, int32 damage) const { return damage; }
    // called before aura apply and after aura unapply
    virtual void OnApply(Aura* aura, bool apply) const {}
    // called during proc eligibility checking
    virtual bool OnCheckProc(Aura* aura) const { return true; }
    // called before proc handler
    virtual SpellAuraProcResult OnProc(Aura* aura, ProcExecutionData& procData) const { return SPELL_AURA_PROC_OK; }
    // called on absorb of this aura
    virtual void OnAbsorb(Aura* aura, int32& currentAbsorb, uint32& reflectedSpellId, int32& reflectDamage, bool& preventedDeath) const {}
    // called on mana shield absorb of this aura
    virtual void OnManaAbsorb(Aura* aura, int32& currentAbsorb) const {}
    // called on death prevention
    virtual void OnAuraDeathPrevention(Aura* aura, int32& remainingDamage) const {}
    // called on aura dispel
    virtual void OnDispel(SpellAuraHolder* holder, Unit* dispeller, uint32 dispellingSpellId, uint32 originalStacks) const {}
    // called on periodic auras which need amount calculation (damage, heal, burn, drain)
    virtual void OnPeriodicCalculateAmount(Aura* aura, uint32& amount) const {}
    // called on periodic spell trigger
    virtual void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const {}
    // called on periodic dummy
    virtual void OnPeriodicDummy(Aura* aura) const {}
    // called on periodic tick end
    virtual void OnPeriodicTickEnd(Aura* aura) const {}
};

class SpellScriptMgr
{
    public:
        static SpellScript* GetSpellScript(uint32 spellId);
        static AuraScript* GetAuraScript(uint32 spellId);

        static void SetSpellScript(std::string scriptName, SpellScript* script);
        static void SetAuraScript(std::string scriptName, AuraScript* script);

        static void LoadScripts();
    private:
        static SpellScript* GetSpellScript(std::string scriptName);
        static AuraScript* GetAuraScript(std::string scriptName);

        static void SetSpellScript(uint32 spellId, SpellScript* script);
        static void SetAuraScript(uint32 spellId, AuraScript* script);

        static std::map<uint32, SpellScript*> m_spellScriptMap;
        static std::map<uint32, AuraScript*> m_auraScriptMap;
        static std::map<std::string, SpellScript*> m_spellScriptStringMap;
        static std::map<std::string, AuraScript*> m_auraScriptStringMap;
};

template <class T, class U>
void RegisterScript(std::string stringName)
{
    static_assert(std::is_base_of<SpellScript, T>::value, "T not derived from SpellScript");
    static_assert(std::is_base_of<AuraScript, U>::value, "T not derived from AuraScript");
    SpellScriptMgr::SetSpellScript(stringName, new T());
    SpellScriptMgr::SetAuraScript(stringName, new U());
}

template <class T>
void RegisterSpellScript(std::string stringName)
{
    static_assert(std::is_base_of<SpellScript, T>::value, "T not derived from SpellScript");
    SpellScriptMgr::SetSpellScript(stringName, new T());
}

template <class U>
void RegisterAuraScript(std::string stringName)
{
    static_assert(std::is_base_of<AuraScript, U>::value, "T not derived from AuraScript");
    SpellScriptMgr::SetAuraScript(stringName, new U());
}

#endif // SPELLSCRIPT_H