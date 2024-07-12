/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANGOS_UNIQUE_TRACKABLE_PTR_H
#define MANGOS_UNIQUE_TRACKABLE_PTR_H

#include <memory>

namespace MaNGOS
{
// C++20 utilities
template <typename T>
inline constexpr bool is_bounded_array_v = std::is_array_v<T> && std::extent_v<T> != 0;

template <typename T>
inline constexpr bool is_unbounded_array_v = std::is_array_v<T> && std::extent_v<T> == 0;

template <typename T>
class unique_trackable_ptr;

template <typename T>
class unique_weak_ptr;

template <typename T>
class unique_strong_ref_ptr;

/**
 * \brief Specialized variant of std::shared_ptr that enforces unique ownership and/or std::unique_ptr with std::weak_ptr capabilities
 * Implementation has the same overhead as a std::shared_ptr, that is, a separate allocation for control block that holds use counters
 * \tparam T Type of held object
 */
template <typename T>
class unique_trackable_ptr
{
public:
    using element_type = T;
    using pointer = T*;

    unique_trackable_ptr() : _ptr() { }

    explicit unique_trackable_ptr(pointer ptr)
        : _ptr(ptr) { }

    template <typename Deleter, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter>, std::is_invocable<Deleter&, T*&>>, int> = 0>
    explicit unique_trackable_ptr(pointer ptr, Deleter deleter)
        : _ptr(ptr, std::move(deleter)) { }

    unique_trackable_ptr(unique_trackable_ptr const&) = delete;

    unique_trackable_ptr(unique_trackable_ptr&& other) noexcept
        : _ptr(std::move(other._ptr)) { }

    template <typename T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
    unique_trackable_ptr(unique_trackable_ptr<T2>&& other) noexcept
        : _ptr(std::move(other)._ptr) { }

    unique_trackable_ptr& operator=(unique_trackable_ptr const&) = delete;

    unique_trackable_ptr& operator=(unique_trackable_ptr&& other) noexcept
    {
        _ptr = std::move(other._ptr);
        return *this;
    }

    template <typename T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
    unique_trackable_ptr& operator=(unique_trackable_ptr<T2>&& other) noexcept
    {
        _ptr = std::move(other)._ptr;
        return *this;
    }

    ~unique_trackable_ptr() = default;

    unique_trackable_ptr& operator=(std::nullptr_t)
    {
        reset();
        return *this;
    }

    void swap(unique_trackable_ptr& other) noexcept
    {
        using std::swap;
        swap(_ptr, other._ptr);
    }

    element_type& operator*() const
    {
        return *_ptr;
    }

    pointer operator->() const
    {
        return _ptr.operator->();
    }

    pointer get() const
    {
        return _ptr.get();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(_ptr);
    }

    void reset()
    {
        _ptr.reset();
    }

    void reset(pointer ptr)
    {
        _ptr.reset(ptr);
    }

    template <class Deleter, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter>, std::is_invocable<Deleter&, T*&>>, int> = 0>
    void reset(pointer ptr, Deleter deleter)
    {
        _ptr.reset(ptr, std::move(deleter));
    }

private:
    template <typename T0>
    friend class unique_trackable_ptr;

    template <typename T0>
    friend class unique_weak_ptr;

    template <typename T0, typename... Args>
    friend std::enable_if_t<!std::is_array_v<T0>, unique_trackable_ptr<T0>> make_unique_trackable(Args&&... args);

    template <typename T0>
    friend std::enable_if_t<is_unbounded_array_v<T0>, unique_trackable_ptr<T0>> make_unique_trackable(std::size_t N);

    template <typename T0>
    friend std::enable_if_t<is_unbounded_array_v<T0>, unique_trackable_ptr<T0>> make_unique_trackable(std::size_t N, std::remove_extent_t<T0> const& val);

    template <typename T0>
    friend std::enable_if_t<is_bounded_array_v<T0>, unique_trackable_ptr<T0>> make_unique_trackable();

    template <typename T0>
    friend std::enable_if_t<is_bounded_array_v<T0>, unique_trackable_ptr<T0>> make_unique_trackable(std::remove_extent_t<T0> const& val);

    std::shared_ptr<element_type> _ptr;
};

/**
 * \brief Trinity::unique_trackable_ptr companion class, replicating what std::weak_ptr is to std::shared_ptr
 * \tparam T Type of held object
 */
template <typename T>
class unique_weak_ptr
{
public:
    using element_type = T;
    using pointer = T*;

    unique_weak_ptr() = default;

    unique_weak_ptr(unique_trackable_ptr<T> const& trackable)
        : _ptr(trackable._ptr) { }

    unique_weak_ptr(unique_weak_ptr const& other) = default;

    template <typename T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
    unique_weak_ptr(unique_weak_ptr<T2> const& other) noexcept
        : _ptr(other._ptr) { }

    unique_weak_ptr(unique_weak_ptr&& other) noexcept = default;

    template <typename T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
    unique_weak_ptr(unique_weak_ptr<T2>&& other) noexcept
        : _ptr(std::move(other)._ptr) { }

    unique_weak_ptr& operator=(unique_trackable_ptr<T> const& trackable)
    {
        _ptr = trackable._ptr;
        return *this;
    }

    unique_weak_ptr& operator=(unique_weak_ptr const& other) = default;

    template <typename T2, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
    unique_weak_ptr& operator=(unique_weak_ptr<T2>&& other)
    {
        _ptr = std::move(other)._ptr;
        return *this;
    }

    unique_weak_ptr& operator=(unique_weak_ptr&& other) noexcept = default;

    ~unique_weak_ptr() = default;

    void swap(unique_weak_ptr& other) noexcept
    {
        using std::swap;
        swap(_ptr, other._ptr);
    }

    bool expired() const
    {
        return _ptr.expired();
    }

    unique_strong_ref_ptr<element_type> lock() const
    {
        return unique_strong_ref_ptr<element_type>(_ptr.lock());
    }

private:
    template <typename T0>
    friend class unique_weak_ptr;

    template <typename T0>
    friend class unique_strong_ref_ptr;

    template <class To, class From>
    friend unique_weak_ptr<To> static_pointer_cast(unique_weak_ptr<From> const& other);

    template <class To, class From>
    friend unique_weak_ptr<To> const_pointer_cast(unique_weak_ptr<From> const& other);

    template <class To, class From>
    friend unique_weak_ptr<To> reinterpret_pointer_cast(unique_weak_ptr<From> const& other);

    template <class To, class From>
    friend unique_weak_ptr<To> dynamic_pointer_cast(unique_weak_ptr<From> const& other);

    std::weak_ptr<element_type> _ptr;
};

/**
 * \brief Result of unique_weak_ptr::lock() function, this class holds a temporary strong reference to held object
 * to prevent it from being deallocated by another thread while it is being actively accessed.
 * This class is non-movable and non-copypable and is intended only for short lived local variables
 * \tparam T Type of held object
 */
template <typename T>
class unique_strong_ref_ptr
{
public:
    using element_type = T;
    using pointer = T*;

    unique_strong_ref_ptr(unique_strong_ref_ptr const&) = delete;
    unique_strong_ref_ptr(unique_strong_ref_ptr&&) = delete;
    unique_strong_ref_ptr& operator=(unique_strong_ref_ptr const&) = delete;
    unique_strong_ref_ptr& operator=(unique_strong_ref_ptr&&) = delete;

    ~unique_strong_ref_ptr() = default;

    element_type& operator*() const
    {
        return *_ptr;
    }

    pointer operator->() const
    {
        return _ptr.operator->();
    }

    pointer get() const
    {
        return _ptr.get();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(_ptr);
    }

    operator unique_weak_ptr<T>() const
    {
        unique_weak_ptr<T> weak;
        weak._ptr = _ptr;
        return weak;
    }

private:
    template <typename T0>
    friend class unique_weak_ptr;

    template <class To, class From>
    friend unique_strong_ref_ptr<To> static_pointer_cast(unique_strong_ref_ptr<From> const& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> static_pointer_cast(unique_strong_ref_ptr<From>&& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> const_pointer_cast(unique_strong_ref_ptr<From> const& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> const_pointer_cast(unique_strong_ref_ptr<From>&& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> reinterpret_pointer_cast(unique_strong_ref_ptr<From> const& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> reinterpret_pointer_cast(unique_strong_ref_ptr<From>&& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> dynamic_pointer_cast(unique_strong_ref_ptr<From> const& other);

    template <class To, class From>
    friend unique_strong_ref_ptr<To> dynamic_pointer_cast(unique_strong_ref_ptr<From>&& other);

    unique_strong_ref_ptr(std::shared_ptr<element_type> ptr) : _ptr(std::move(ptr)) { }

    std::shared_ptr<element_type> _ptr;
};

// Ugly macros that will become unneccessary with C++20 operator<=>
#define UNIQUE_TRACKABLE_COMPARISON_OPERATOR(cls, op) \
template <typename T1, typename T2> \
bool operator op (cls <T1> const& left, cls <T2> const& right) { return left.get() op right.get(); }

#define UNIQUE_TRACKABLE_COMPARISON_OPERATOR_NULLPTR(cls, op) \
template <typename T1> \
bool operator op (cls <T1> const& left, std::nullptr_t) { return left.get() op static_cast<typename cls <T1>::element_type*>(nullptr); }\
template <typename T1> \
bool operator op (std::nullptr_t, cls <T1> const& right) { return static_cast<typename cls <T1>::element_type*>(nullptr) op right.get(); }

#define UNIQUE_TRACKABLE_COMPARISON_OPERATORS(STAMPER, cls) \
    STAMPER(cls, ==) \
    STAMPER(cls, !=) \
    STAMPER(cls, <)  \
    STAMPER(cls, >=) \
    STAMPER(cls, >)  \
    STAMPER(cls, <=)

#define UNIQUE_TRACKABLE_COMPARISON_OPERATORS_FOR_CLASS(cls) \
    UNIQUE_TRACKABLE_COMPARISON_OPERATORS(UNIQUE_TRACKABLE_COMPARISON_OPERATOR, cls) \
    UNIQUE_TRACKABLE_COMPARISON_OPERATORS(UNIQUE_TRACKABLE_COMPARISON_OPERATOR_NULLPTR, cls)

// unique_trackable_ptr funcions
UNIQUE_TRACKABLE_COMPARISON_OPERATORS_FOR_CLASS(unique_trackable_ptr)

template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, unique_trackable_ptr<T>> make_unique_trackable(Args&&... args)
{
    unique_trackable_ptr<T> ptr;
    ptr._ptr = std::make_shared<T>(std::forward<Args>(args)...);
    return ptr;
}

template <typename T>
std::enable_if_t<is_unbounded_array_v<T>, unique_trackable_ptr<T>> make_unique_trackable(std::size_t N)
{
    unique_trackable_ptr<T> ptr;
    ptr._ptr = std::make_shared<T>(N);
    return ptr;
}

template <typename T>
std::enable_if_t<is_unbounded_array_v<T>, unique_trackable_ptr<T>> make_unique_trackable(std::size_t N, std::remove_extent_t<T> const& val)
{
    unique_trackable_ptr<T> ptr;
    ptr._ptr = std::make_shared<T>(N, val);
    return ptr;
}

template <typename T>
std::enable_if_t<is_bounded_array_v<T>, unique_trackable_ptr<T>> make_unique_trackable()
{
    unique_trackable_ptr<T> ptr;
    ptr._ptr = std::make_shared<T>();
    return ptr;
}

template <typename T>
std::enable_if_t<is_bounded_array_v<T>, unique_trackable_ptr<T>> make_unique_trackable(std::remove_extent_t<T> const& val)
{
    unique_trackable_ptr<T> ptr;
    ptr._ptr = std::make_shared<T>(val);
    return ptr;
}

// unique_weak_ptr funcions

template <class To, class From>
unique_weak_ptr<To> static_pointer_cast(unique_weak_ptr<From> const& other)
{
    unique_weak_ptr<To> to;
    to._ptr = std::static_pointer_cast<To>(other._ptr.lock());
    return to;
}

template <class To, class From>
unique_weak_ptr<To> const_pointer_cast(unique_weak_ptr<From> const& other)
{
    unique_weak_ptr<To> to;
    to._ptr = std::const_pointer_cast<To>(other._ptr.lock());
    return to;
}

template <class To, class From>
unique_weak_ptr<To> reinterpret_pointer_cast(unique_weak_ptr<From> const& other)
{
    unique_weak_ptr<To> to;
    to._ptr = std::reinterpret_pointer_cast<To>(other._ptr.lock());
    return to;
}

template <class To, class From>
unique_weak_ptr<To> dynamic_pointer_cast(unique_weak_ptr<From> const& other)
{
    unique_weak_ptr<To> to;
    to._ptr = std::dynamic_pointer_cast<To>(other._ptr.lock());
    return to;
}

// unique_strong_ref_ptr funcions
UNIQUE_TRACKABLE_COMPARISON_OPERATORS_FOR_CLASS(unique_strong_ref_ptr)

template <class To, class From>
unique_strong_ref_ptr<To> static_pointer_cast(unique_strong_ref_ptr<From> const& other)
{
    return unique_strong_ref_ptr<To>(std::static_pointer_cast<To>(other._ptr));
}

template <class To, class From>
unique_strong_ref_ptr<To> static_pointer_cast(unique_strong_ref_ptr<From>&& other)
{
    return unique_strong_ref_ptr<To>(std::static_pointer_cast<To>(std::move(other._ptr)));
}

template <class To, class From>
unique_strong_ref_ptr<To> const_pointer_cast(unique_strong_ref_ptr<From> const& other)
{
    return unique_strong_ref_ptr<To>(std::const_pointer_cast<To>(other._ptr));
}

template <class To, class From>
unique_strong_ref_ptr<To> const_pointer_cast(unique_strong_ref_ptr<From>&& other)
{
    return unique_strong_ref_ptr<To>(std::const_pointer_cast<To>(std::move(other._ptr)));
}

template <class To, class From>
unique_strong_ref_ptr<To> reinterpret_pointer_cast(unique_strong_ref_ptr<From> const& other)
{
    return unique_strong_ref_ptr<To>(std::reinterpret_pointer_cast<To>(other._ptr));
}

template <class To, class From>
unique_strong_ref_ptr<To> reinterpret_pointer_cast(unique_strong_ref_ptr<From>&& other)
{
    return unique_strong_ref_ptr<To>(std::reinterpret_pointer_cast<To>(std::move(other._ptr)));
}

template <class To, class From>
unique_strong_ref_ptr<To> dynamic_pointer_cast(unique_strong_ref_ptr<From> const& other)
{
    return unique_strong_ref_ptr<To>(std::dynamic_pointer_cast<To>(other._ptr));
}

template <class To, class From>
unique_strong_ref_ptr<To> dynamic_pointer_cast(unique_strong_ref_ptr<From>&& other)
{
    return unique_strong_ref_ptr<To>(std::dynamic_pointer_cast<To>(std::move(other._ptr)));
}
}

#endif // MANGOS_UNIQUE_TRACKABLE_PTR_H
