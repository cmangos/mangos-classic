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

#ifndef MANGOSSERVER_ENUM_FLAG_H
#define MANGOSSERVER_ENUM_FLAG_H

#include <type_traits>

template<typename T>
constexpr bool IsEnumFlag(T) { return false; }

#define DEFINE_ENUM_FLAG(enumType) constexpr bool IsEnumFlag(enumType) { return std::is_enum_v<enumType>; }

template<typename T, typename = std::enable_if_t<IsEnumFlag(T{})>>
inline constexpr T operator&(T left, T right)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) & static_cast<std::underlying_type_t<T>>(right));
}

template<typename T, typename = std::enable_if_t<IsEnumFlag(T{})>>
inline constexpr T& operator&=(T& left, T right)
{
    return left = left & right;
}

template<typename T, typename = std::enable_if_t<IsEnumFlag(T{})>>
inline constexpr T operator|(T left, T right)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(left) | static_cast<std::underlying_type_t<T>>(right));
}

template<typename T, typename = std::enable_if_t<IsEnumFlag(T{})>>
inline constexpr T& operator|=(T& left, T right)
{
    return left = left | right;
}

template<typename T, typename = std::enable_if_t<IsEnumFlag(T{})>>
inline constexpr T operator~(T value)
{
    return static_cast<T>(~static_cast<std::underlying_type_t<T>>(value));
}

#endif
