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

#ifndef CAV_TUPLISH_TUPLE_HPP
#define CAV_TUPLISH_TUPLE_HPP

#include "../tuplish/type_map.hpp"

namespace cav {

/////////////////////////////////////////////////////////////////////////
///////////////////////////////// TUPLE /////////////////////////////////
/////////////////////////////////////////////////////////////////////////
template <typename V, typename Tag = UNIQUE_TYPE>
struct tuple_elem {
    using key_t   = Tag;
    using value_t = V;

    [[no_unique_address]] V value;
    constexpr void operator[](Tag) = delete;
};

template <typename... Ts>
struct tuple : type_map<tuple_elem<Ts>...> {};

template <typename... Ts>
tuple(Ts...) -> tuple<Ts...>;

#ifdef CAV_COMP_TESTS
namespace {
    constexpr inline auto tp0 = tuple<>{};
    constexpr inline auto tp1 = tuple<int, float>{1, 2.0};
    constexpr inline auto tp2 = tuple<int, float[10]>{3, {}};
    constexpr inline auto tp3 = tuple<int>{4};
    constexpr inline auto tp4 = tuple<>{};
    constexpr inline auto tp5 = tuple<int, int, int, int, int>{1, 2, 3, 4, 5};

    CAV_PASS(sizeof(tp0) == 1);
    CAV_PASS(sizeof(tp1) == 8);
    CAV_PASS(sizeof(tp2) == 44);
    CAV_PASS(sizeof(tp3) == 4);
    CAV_PASS(sizeof(tp4) == 1);
    CAV_PASS(tp1[0_ct] == 1);
    CAV_PASS(tp1[ct_v<1_uz>] == 2.0);
    CAV_PASS(tp2[1_ct][9] == 0);
    CAV_PASS(tp3[ct_v<std::size_t{}>] == 4);

    CAV_BLOCK_PASS(auto x = std::array<int, 5>(tp5); assert(x[1] == 2););
}  // namespace
#endif

}  // namespace cav

#endif /* CAV_EXPERIMENTAL_TUPLISH_TUPLE_HPP */
