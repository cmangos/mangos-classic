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

#ifndef MANGOSSERVER_CONVERTIBLE_ENUM_FLAG_H
#define MANGOSSERVER_CONVERTIBLE_ENUM_FLAG_H

#include "Platform/Define.h"
#include "Globals/EnumFlag.h"

enum class FlagConvertibleEnum : uint32
{

};

DEFINE_ENUM_FLAG(FlagConvertibleEnum);

enum class FlagConvertibleEnum64 : uint64
{
};

DEFINE_ENUM_FLAG(FlagConvertibleEnum64);

#define DEFINE_FLAG_CONVERTIBLE_ENUM(enumType) constexpr bool IsFlagConvertibleEnum(enumType) { return std::is_enum_v<enumType>; }
#define DEFINE_FLAG_CONVERTIBLE_ENUM_64(enumType) constexpr bool IsFlagConvertibleEnum64(enumType) { return std::is_enum_v<enumType>; }

template <typename T, typename = std::enable_if_t<IsFlagConvertibleEnum(T {})>>
constexpr FlagConvertibleEnum const convertEnumToFlag(T item)
{
    return FlagConvertibleEnum(uint32(1) << static_cast<uint32>(static_cast<std::underlying_type_t<T>>(item) - 1));
}

template <typename T, typename = std::enable_if_t<IsFlagConvertibleEnum64(T {})>>
constexpr FlagConvertibleEnum64 const convertEnumToFlag(T item)
{
    return FlagConvertibleEnum64(uint64(1) << static_cast<uint64>(static_cast<std::underlying_type_t<T>>(item) - 1));
}

template <std::convertible_to<uint32> T>
constexpr uint32 const convertEnumToFlag(T item)
{
    return 1 << (item - 1);
}

template <std::convertible_to<uint32> T, typename... Targs>
constexpr uint32 const convertEnumToFlag(T item, Targs&&... fargs)
{
    return convertEnumToFlag(item) | convertEnumToFlag(std::forward<Targs>(fargs)...);
}

#endif
