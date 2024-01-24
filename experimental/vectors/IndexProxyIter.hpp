// Copyright (c) 2024 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef CAV_VECTORS_INDEXPROXYITER_HPP
#define CAV_VECTORS_INDEXPROXYITER_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>

#include "cav/comptime/syntactic_sugars.hpp"
#include "cav/comptime/test.hpp"

namespace cav {

template <typename T>
concept indexable = requires(T cont) {
    { cont.size() } -> std::integral;
    { cont[0] };
};

template <typename ContT>
struct IndexProxyIter {
    using self              = IndexProxyIter;
    using iterator_category = typename std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;
    using value_type        = typename ContT::value_type;
    using pointer           = value_type*;
    using reference         = value_type&;

    std::size_t idx       = 0;
    ContT*      container = nullptr;

    [[nodiscard]] constexpr operator IndexProxyIter<ContT const>() const {
        return {idx, container};
    }

    [[nodiscard]] constexpr decl_auto operator*() const {
        return container->operator[](idx);
    }

    [[nodiscard]] constexpr decl_auto operator[](ptrdiff_t n) const {
        return (*container)[idx + n];
    }

    constexpr self& operator++() {
        ++idx;
        return *this;
    }

    constexpr self operator++(int) {
        ++idx;
        return {idx - 1, container};
    }

    constexpr self& operator--() {
        --idx;
        return *this;
    }

    constexpr self operator--(int) {
        --idx;
        return {idx + 1, container};
    }

    constexpr self& operator+=(ptrdiff_t n) {
        idx += n;
        return *this;
    }

    constexpr self& operator-=(ptrdiff_t n) {
        idx -= n;
        return *this;
    }

    [[nodiscard]] constexpr ptrdiff_t operator-(self other) const {
        assert(container == other.container);
        return idx - other.idx;
    }

    [[nodiscard]] constexpr bool operator==(self other) const {
        assert(container == other.container);
        return idx == other.idx;
    }

    [[nodiscard]] constexpr auto operator<=>(self other) const {
        assert(container == other.container);
        return idx <=> other.idx;
    }
};

template <typename ContT>
IndexProxyIter(size_t, ContT*) -> IndexProxyIter<ContT>;

template <indexable ContT>
constexpr IndexProxyIter<ContT> operator+(IndexProxyIter<ContT> iter, ptrdiff_t n) {
    return iter += n;
}

template <indexable ContT>
constexpr IndexProxyIter<ContT> operator+(ptrdiff_t n, IndexProxyIter<ContT> iter) {
    return iter += n;
}

template <indexable ContT>
constexpr IndexProxyIter<ContT> operator-(IndexProxyIter<ContT> iter, ptrdiff_t n) {
    return iter -= n;
}

template <indexable ContT>
constexpr IndexProxyIter<ContT> operator-(ptrdiff_t n, IndexProxyIter<ContT> iter) {
    return iter -= n;
}
}  // namespace cav

#ifdef CAV_COMP_TESTS
#include <vector>

namespace cav { namespace {
    CAV_PASS(std::random_access_iterator<IndexProxyIter<std::vector<int>>>);

    CAV_BLOCK_PASS({
        auto vec = std::vector<int>{0, 1, 2, 3, 4};
        auto beg = IndexProxyIter(0, &vec);
        auto end = IndexProxyIter(5, &vec);
        for (auto it = beg; int n : vec) {
            assert(*it == n);
            assert(it < end);
            assert(std::distance(beg, it) == n);
            ++it;
        }
    });

}  // namespace
}  // namespace cav
#endif

#endif /* CAV_VECTORS_INDEXPROXYITER_HPP */
