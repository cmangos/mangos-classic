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

#include "Mod.h"

#include "Log.h"
#include "SystemConfig.h"
#include "Policies/SingletonImp.h"


INSTANTIATE_SINGLETON_1( ModClass );

ModClass::ModClass()
{
    for(int i = 0; i < MODCONFIG_UINT32_VALUE_COUNT; ++i)
        m_modConfigUint32Values[i] = 0;

    for(int i = 0; i < MODCONFIG_INT32_VALUE_COUNT; ++i)
        m_modConfigInt32Values[i] = 0;

    for(int i = 0; i < MODCONFIG_FLOAT_VALUE_COUNT; ++i)
        m_modConfigFloatValues[i] = 0.0f;

    for(int i = 0; i < MODCONFIG_BOOL_VALUE_COUNT; ++i)
        m_modConfigBoolValues[i] = false;
}

ModClass::~ModClass()
{
}

void ModClass::ModInit()
{
    LoadModConfSettings();
}

void ModClass::LoadModConfSettings(bool reload)
{
    bool loaded = true;
    if(reload)
    {
        if(!ModConfig.Reload())
        {
            sLog.outError("Mod settings reload fail: can't read settings from %s.",ModConfig.GetFilename().c_str());
            loaded = false;
            return;
        }
    } 
    else
    {
        //Get configuration file
        if (!ModConfig.SetSource(_MODS_CONFIG))
        {
            loaded = false;
            error_log("MODS: Unable to open configuration file. Configuration values will use default.");
        }
        else
            outstring_log("MODS: Using configuration file %s",_MODS_CONFIG);
    }

        ///- Read the version of the configuration file and warn the user in case of emptiness or mismatch

    if(loaded)
    {
       if (ModConfig.GetIntDefault("ConfVersion", 0) < _MODSCONFVERSION)
        {
            sLog.outError("*****************************************************************************");
            sLog.outError(" WARNING: Your mod.conf version indicates your conf file is out of date!");
            sLog.outError("          Please check for updates, as your current default values may cause");
            sLog.outError("          unexpected behavior.");
            sLog.outError("*****************************************************************************");
            Log::WaitBeforeContinueIfNeed();
        }
    }

    setModConfig(MODCONFIG_BOOL_TBC_DIMINISHING_DURATION,"Mod.Tbc.DiminishingDuration",false);
}
