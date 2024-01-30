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

#ifndef CAV_TUPLISH_TYPE_SET_HPP
#define CAV_TUPLISH_TYPE_SET_HPP

#include "../tuplish/type_map.hpp"

namespace cav {
////////////////////////////////////////////////////////////////////////
/////////////////////////////// TYPE SET ///////////////////////////////
////////////////////////////////////////////////////////////////////////

template <typename... Ts>
struct type_set : type_map<map_elem<Ts, Ts>...> {};

template <typename... Ts>
type_set(Ts...) -> type_set<Ts...>;

#ifdef CAV_COMP_TESTS
namespace {
    constexpr inline auto ts1 = type_set<int, float>{1, 2.0};
    constexpr inline auto ts2 = type_set<int, float[10]>{3, {}};
    constexpr inline auto ts3 = type_set<int>{4};
    constexpr inline auto ts4 = type_set<>{};

    CAV_PASS(sizeof(ts1) == 8);
    CAV_PASS(sizeof(ts2) == 44);
    CAV_PASS(sizeof(ts3) == 4);
    CAV_PASS(sizeof(ts4) == 1);
    CAV_PASS(ts1["std::int32_t"_cs] == 1);
    CAV_PASS(ts1[tag<float>] == 2.0);
    CAV_PASS(ts2[tag<float[10]>][9] == 0);
    CAV_PASS(ts3[tag<int>] == 4);
}  // namespace
#endif

}  // namespace cav

#endif /* CAV_TUPLISH_TYPE_SET_HPP */
