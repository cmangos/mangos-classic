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

#ifndef CONFIG_H
#define CONFIG_H

#include "Common.h"
#include "Policies/Singleton.h"
#include "Platform/Define.h"
#include <mutex>

#include <string>
#include <unordered_map>

class Config
{
    private:
        std::string m_filename;
        std::unordered_map<std::string, std::string> m_entries; // keys are converted to lower case.  values cannot be.

    public:
        bool SetSource(const std::string& file);
        bool Reload();

        bool IsSet(const std::string& name) const;

        const std::string GetStringDefault(const std::string& name, const std::string& def = "") const;
        bool GetBoolDefault(const std::string& name, bool def) const;
        int32 GetIntDefault(const std::string& name, int32 def) const;
        float GetFloatDefault(const std::string& name, float def) const;

        const std::string& GetFilename() const { return m_filename; }
        std::mutex m_configLock;
};

#define sConfig MaNGOS::Singleton<Config>::Instance()

#endif
