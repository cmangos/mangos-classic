/*
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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

#ifndef _MOD_H
#define _MOD_H

#include "Common.h"
#include "../DBCEnums.h"
#include "../SharedDefines.h"
#include "../SpellAuraDefines.h"
#include "Config/Config.h"

enum ModConfigUint32Values
{
    MODCONFIG_UINT32_EMPTY = 0,  //overwrite with new conf
    MODCONFIG_UINT32_VALUE_COUNT
};

enum ModConfigInt32Values
{
    MODCONFIG_INT32_EMPTY = 0,  //overwrite with new conf
    MODCONFIG_INT32_VALUE_COUNT
};

enum ModConfigFloatValues
{
    MODCONFIG_FLOAT_EMPTY = 0,  //overwrite with new conf
    MODCONFIG_FLOAT_VALUE_COUNT
};

enum ModConfigBoolValues
{
    MODCONFIG_BOOL_TBC_DIMINISHING_DURATION = 0,
    MODCONFIG_BOOL_VALUE_COUNT
};

class Aura;
class Spell;
struct SpellEntry;
class Unit;

class MANGOS_DLL_SPEC ModClass
{
	public:
		ModClass();
		~ModClass();

        void ModInit();
        void LoadModConfSettings(bool reload = false);

        // Getters and Setters
        void setModConfig(ModConfigUint32Values index, char const* fieldname, uint32 defvalue) { m_modConfigUint32Values[index] = ModConfig.GetIntDefault(fieldname,defvalue); }
        uint32 getModConfig(ModConfigUint32Values index) const { return m_modConfigUint32Values[index]; }

        void setModConfig(ModConfigInt32Values index, char const* fieldname, int32 defvalue) { m_modConfigInt32Values[index] = ModConfig.GetIntDefault(fieldname,defvalue); }
        int32 getModConfig(ModConfigInt32Values index) const { return m_modConfigInt32Values[index]; }

        void setModConfig(ModConfigFloatValues index, char const* fieldname, float defvalue) { m_modConfigFloatValues[index] = ModConfig.GetFloatDefault(fieldname,defvalue); }
        float getModConfig(ModConfigFloatValues rate) const { return m_modConfigFloatValues[rate]; }

        void setModConfig(ModConfigBoolValues index, char const* fieldname, bool defvalue) { m_modConfigBoolValues[index] = ModConfig.GetBoolDefault(fieldname,defvalue); }
        bool getModConfig(ModConfigBoolValues index) const { return m_modConfigBoolValues[index]; }

        // ==== Spell mods ==== //
        // you can add modifications, workarounds and hack code for spell prepare
        void spellPrepare(Spell* spell,Unit *caster);
        // you can add modifications, workarounds and hack code for spell effect
        void spellEffect(Spell* spell, uint8 eff, SpellEffectIndex i);
        void auraApplyModifier(Aura* aura,AuraType aType,bool apply,bool real);
        void getSpellCastTime(SpellEntry const* spellInfo, Spell const* spell, int32 &castTime);

        // ==== Unit mods ==== //

        void applyDiminishingToDuration(Unit* unit,Unit* caster,int32 &duration, DiminishingGroup group);

	protected:

	private:
        Config ModConfig;

        uint32 m_modConfigUint32Values[MODCONFIG_UINT32_VALUE_COUNT];
        int32 m_modConfigInt32Values[MODCONFIG_INT32_VALUE_COUNT];
        float m_modConfigFloatValues[MODCONFIG_FLOAT_VALUE_COUNT];
        bool m_modConfigBoolValues[MODCONFIG_BOOL_VALUE_COUNT];
};


#define sMod MaNGOS::Singleton<ModClass>::Instance()

#endif

