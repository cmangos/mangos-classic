/*
 * MIT License
 *
 * Copyright (c) 2018 namreeb (legal@namreeb.org) http://github.com/namreeb/useful_containers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

// nam::cyclic is a cyclical, fixed sized ring buffer, similair to boost::circular_buffer which relies on std::array<>

#ifndef __CYCLIC_HPP_
#define __CYCLIC_HPP_

#include <array>
#include <memory>
#include <utility>
#include <type_traits>

namespace nam
{
template <typename CyclicT, bool is_const>
class CyclicIterator
{
private:
    friend CyclicT;
    friend class CyclicIterator<CyclicT, !is_const>;

    std::conditional_t<is_const, const CyclicT, CyclicT>* _me;
    std::size_t _idx;

    CyclicIterator(std::conditional_t<is_const, const CyclicT, CyclicT>* container, std::size_t idx) noexcept : _me(container), _idx(idx) {}

public:
    using value_type = typename CyclicT::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<is_const, const value_type, value_type>*;
    using reference = std::conditional_t<is_const, const value_type, value_type>&;
    using iterator_category = std::input_iterator_tag;

    reference operator*() const
    {
        return (*_me)[_idx];
    }

    bool operator == (CyclicIterator const &other) const noexcept
    {
        return _me == other._me && _idx == other._idx;
    }

    bool operator != (CyclicIterator const &other) const noexcept
    {
        return !(*this == other);
    }

    CyclicIterator& operator++()
    {
        ++_idx;
        return *this;
    }

    CyclicIterator& operator =(const CyclicIterator &other)
    {
        _me = other._me;
        _idx = other._idx;
        return *this;
    }
};

template <typename T, std::size_t N>
class cyclic
{
private:
    bool _filled;
    std::size_t _curr;
    std::array<T, N> _raw;

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = CyclicIterator<cyclic, false>;
    using const_iterator = CyclicIterator<cyclic const, true>;

    cyclic() : _filled(false), _curr(0u) {}

    void push_back(const_reference val) noexcept
    {
        _raw[_curr] = val;

        if (_curr == N - 1)
        {
            _filled = true;
            _curr = 0;
        }
        else
            ++_curr;
    }

    void push_back(T&& val) noexcept
    {
        emplace_back(std::move(val));
    }

    template <class... Args>
    void emplace_back(Args&&... args) noexcept
    {
        new (&_raw[_curr]) T(std::forward<Args>(args)...);

        if (_curr == N - 1)
        {
            _filled = true;
            _curr = 0;
        }
        else
            ++_curr;
    }

    void clear() noexcept
    {
        _filled = false;
        _curr = 0;
    }

    std::size_t size() const noexcept { return _filled ? N : _curr; }
    bool empty() const noexcept { return !_filled && !_curr; }

    reference operator[](std::size_t n)
    {
        // we don't want to use modulo here because not all values of n are considered safe
        if (_filled && (n += _curr) >= N)
            n -= N;

        return _raw[n];
    }

    const_reference operator[](std::size_t n) const
    {
        // we don't want to use modulo here because not all values of n are considered safe
        if (_filled && (n += _curr) >= N)
            n -= N;

        return _raw[n];
    }

    iterator begin() noexcept
    {
        return iterator(this, static_cast<std::size_t>(0));
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(this, static_cast<std::size_t>(0));
    }

    iterator end() noexcept
    {
        return iterator(this, _filled ? N : _curr);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(this, _filled ? N : _curr);
    }
};
}
#endif /* !__CYCLIC_HPP_ */