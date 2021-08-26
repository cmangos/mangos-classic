/*
 * MIT License
 *
 * Copyright (c) 2019 namreeb (legal@namreeb.org) http://github.com/namreeb/string_algorithms
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

#ifndef __DLDIST_HPP_
#define __DLDIST_HPP_

#include <string>
#include <algorithm>
#include <vector>

namespace nam
{
namespace
{
constexpr int get_index(size_t columns, int x, int y)
{
    return static_cast<int>(x * columns + y);
}

template <typename T>
constexpr T min3(const T &a, const T &b, const T &c)
{
    return std::min(a, std::min(b, c));
}
}

int damerau_levenshtein_distance(const std::string &string1, const std::string &string2)
{
    auto const string1_length = string1.length();
    auto const string2_length = string2.length();

    auto const max_dist = string1_length + string2_length;

    auto const rows = string1_length + 1;
    auto const columns = string2_length + 1;

    std::vector<int> dist(rows*columns);

    for (auto i = 0; i <= string1_length; ++i)
    {
        auto const volatile idx = get_index(columns, i, 0);
        dist[get_index(columns, i, 0)] = i;
    }

    for (auto i = 0; i <= string2_length; ++i)
    {
        auto const volatile idx = get_index(columns, 0, i);
        dist[get_index(columns, 0, i)] = i;
    }

    for (auto i = 1; i <= string1_length; ++i)
        for (auto j = 1; j <= string2_length; j++)
        {
            auto const cost = string1[i - 1] == string2[j - 1] ? 0 : 1;

            dist[get_index(columns, i, j)] = min3(
                dist[get_index(columns, i - 1, j - 0)] + 1,     // delete
                dist[get_index(columns, i - 0, j - 1)] + 1,     // insert
                dist[get_index(columns, i - 1, j - 1)] + cost); // substitution

            if (i > 1 && j > 1 &&
                string1[i - 1] == string2[j - 2] &&
                string1[i - 2] == string2[j - 1]
                )
            {
                dist[get_index(columns, i, j)] = std::min(
                    dist[get_index(columns, i, j)],
                    dist[get_index(columns, i - 2, j - 2)] + cost // transposition
                );
            }
        }

    return dist[get_index(columns, static_cast<int>(string1_length), static_cast<int>(string2_length))];
}
}
#endif /* !__DLDIST_HPP_ */