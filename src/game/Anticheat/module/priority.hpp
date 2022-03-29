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

// nam::priority is a fixed sized array in which each element has a priority.  insertions ensure that only
// the highest priority elements are retained.

#ifndef __PRIORITY_HPP_
#define __PRIORITY_HPP_

#include <array>
#include <type_traits>

namespace nam
{
template <typename PriorityT, bool is_const>
class PriorityIterator
{
private:
    friend PriorityT;
    friend class PriorityIterator<PriorityT, !is_const>;

    std::conditional_t<is_const, const PriorityT, PriorityT>* _me;
    std::size_t _idx;

    PriorityIterator(std::conditional_t<is_const, const PriorityT, PriorityT>* container, std::size_t idx) noexcept : _me(container), _idx(idx) {}

public:
    using value_type = typename PriorityT::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<is_const, const value_type, value_type>*;
    using reference = std::conditional_t<is_const, const value_type, value_type>&;
    using iterator_category = std::input_iterator_tag;

    reference operator*() const
    {
        return (*_me)[_idx];
    }

    bool operator == (PriorityIterator const &other) const noexcept
    {
        return _me == other._me && _idx == other._idx;
    }

    bool operator != (PriorityIterator const &other) const noexcept
    {
        return !(*this == other);
    }

    PriorityIterator& operator++()
    {
        ++_idx;
        return *this;
    }

    PriorityIterator& operator =(const PriorityIterator &other)
    {
        _me = other._me;
        _idx = other._idx;
        return *this;
    }
};

template <typename T, typename P, std::size_t N>
class priority
{
private:
    std::size_t _curr;
    std::array<std::pair<P, T>, N> _raw;

    std::size_t find_idx(P prio) const noexcept
    {
        // TODO: binary search
        for (auto i = 0u; i < _curr; ++i)
            if (_raw[i].first < prio)
                return i;

        return _curr;
    }

    void shift(std::size_t idx) noexcept
    {
        for (auto i = N - 1; i > idx; --i)
            _raw[i] = _raw[i - 1];
    }

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = PriorityIterator<priority, false>;
    using const_iterator = PriorityIterator<priority, true>;

    priority() : _curr(0u) {}

    template <class... Args>
    void emplace(P prio, Args&&... args) noexcept
    {
        auto const idx = find_idx(prio);

        if (idx == N)
            return;

        shift(idx);
        _raw[idx] = std::make_pair(prio, std::forward<Args>(args)...);

        if (_curr < N)
            ++_curr;
    }

    void insert(P prio, const_reference elem) noexcept
    {
        emplace(prio, elem);
    }

    void clear() noexcept
    {
        _curr = 0;
    }

    std::size_t size() const noexcept { return _curr; }
    bool empty() const noexcept { return !_curr; }

    reference operator[](std::size_t n)
    {
        return _raw[n].second;
    }

    const_reference operator[](std::size_t n) const
    {
        return _raw[n].second;
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
        return iterator(this, _curr);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(this, _curr);
    }
};
}

#endif /* !__PRIORITY_HPP_ */