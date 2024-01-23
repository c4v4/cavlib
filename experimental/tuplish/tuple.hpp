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

#ifndef CAV_EXPERIMENTAL_TUPLISH_TUPLE_HPP
#define CAV_EXPERIMENTAL_TUPLISH_TUPLE_HPP

#include <bits/utility.h>

#include <type_traits>

#include "../../include/cav/comptime/call_utils.hpp"
#include "../../include/cav/comptime/mp_base.hpp"
#include "../../include/cav/comptime/syntactic_sugars.hpp"
#include "../../include/cav/comptime/type_name.hpp"

namespace cav {

template <typename>
struct tag_type {};

template <typename T>
constexpr auto tag = tag_type<T>{};

////////////////////////////////////////////////////////////////////////
/////////////////////////////// TYPE MAP ///////////////////////////////
////////////////////////////////////////////////////////////////////////

template <typename K, typename V>
struct map_elem {
    [[no_unique_address]] V value;

    [[nodiscard]] constexpr V&& operator[](tag_type<K> /*k*/) && {
        return std::move(value);
    }

    [[nodiscard]] constexpr V const& operator[](tag_type<K> /*k*/) const& {
        return value;
    }

    [[nodiscard]] constexpr V& operator[](tag_type<K> /*k*/) & {
        return value;
    }

    [[nodiscard]] constexpr V&& operator[](ct<type_name<K>::name> /*k*/) && {
        return std::move(value);
    }

    [[nodiscard]] constexpr V const& operator[](ct<type_name<K>::name> /*k*/) const& {
        return value;
    }

    [[nodiscard]] constexpr V& operator[](ct<type_name<K>::name> /*k*/) & {
        return value;
    }
};

template <typename... Ts>
struct type_map : Ts... {
    using Ts::operator[]...;

    [[nodiscard]] static constexpr size_t size() {
        return sizeof...(Ts);
    }

    [[nodiscard]] constexpr decl_auto reduce(auto&& fn) && {
        return FWD(fn)(std::move(Ts::value)...);
    }

    [[nodiscard]] constexpr decl_auto reduce(auto&& fn) const& {
        return FWD(fn)(Ts::value...);
    }

    [[nodiscard]] constexpr decl_auto reduce(auto&& fn) & {
        return FWD(fn)(Ts::value...);
    }

    // return true if early exit happens, can work as "any"
    constexpr bool for_each(auto&& fn) && {
        return (ret_bool_or_false(FWD(fn), std::move(Ts::value)) || ...);
    }

    constexpr bool for_each(auto&& fn) const& {
        return (ret_bool_or_false(FWD(fn), Ts::value) || ...);
    }

    constexpr bool for_each(auto&& fn) & {
        return (ret_bool_or_false(FWD(fn), Ts::value) || ...);
    }

    // return true if fn has been called (otherwise i was larger than size())
    constexpr bool visit_idx(size_t i, auto&& fn) && {
        return reduce([&](auto&&... x) { return ((i-- == 0 && (FWD(fn)(FWD(x)), true)) || ...); });
    }

    constexpr bool visit_idx(size_t i, auto&& fn) const& {
        return reduce([&](auto&... x) { return ((i-- == 0 && (FWD(fn)(x), true)) || ...); });
    }

    constexpr bool visit_idx(size_t i, auto&& fn) & {
        return reduce([&](auto&... x) { return ((i-- == 0 && (FWD(fn)(x), true)) || ...); });
    }

    template <typename T>
    [[nodiscard]] explicit constexpr operator T() const& {
        return {Ts::value...};
    }

    template <typename T>
    [[nodiscard]] explicit constexpr operator T() && {
        return {std::move(Ts::value)...};
    }

    /// Here to take advantage of __type_pack_element when available
    template <value_wrap I>
    [[nodiscard]] constexpr decl_auto operator[](ct<I> /*k*/) && {
        return std::move(nth_type_t<I, Ts...>::value);
    }

    template <value_wrap I>
    [[nodiscard]] constexpr auto const& operator[](ct<I> /*k*/) const& {
        return nth_type_t<I, Ts...>::value;
    }

    template <value_wrap I>
    [[nodiscard]] constexpr auto& operator[](ct<I> /*k*/) & {
        return nth_type_t<I, Ts...>::value;
    }
};

#ifdef CAV_COMP_TESTS
namespace {
    constexpr inline auto tm1 = type_map<map_elem<int, int>, map_elem<ct<2>, float>>{1, 2.0};
    constexpr inline auto tm2 = type_map<map_elem<ct<5>, int>, map_elem<float, float[10]>>{3, {}};
    constexpr inline auto tm3 = type_map<map_elem<int, int>>{4};
    constexpr inline auto tm4 = type_map<>{};

    CAV_PASS(sizeof(tm1) == 8);
    CAV_PASS(sizeof(tm2) == 44);
    CAV_PASS(sizeof(tm3) == 4);
    CAV_PASS(sizeof(tm4) == 1);
    CAV_PASS(tm1["int"_cs] == 1);
    CAV_PASS(tm1[tag<ct<2>>] == 2.0);
    CAV_PASS(tm2[tag<float>][9] == 0);
    CAV_PASS(tm3[tag<int>] == 4);

    CAV_PASS(tm1.visit_idx(0, [](auto x) { assert(x == 1); }));
    CAV_PASS(tm1.visit_idx(1, [](auto x) { assert(x == 2.0); }));
    CAV_PASS(!tm1.visit_idx(2, [](auto x) { assert(x == 2.0); }));  // out of bounds
    CAV_PASS(tm1.for_each([](auto x) { return x > 0; }));           // any > 0? true
    CAV_PASS(!tm1.for_each([](auto x) { return x == 0; }));         // any == 0? false

    CAV_BLOCK_PASS(int x = static_cast<int>(tm3); assert(x == 4));
    CAV_BLOCK_PASS({
        int c = 0;
        assert(tm2.for_each([&c](auto) { return ++c == 1; }));  // found at first
        assert(c == 1);
    });
    CAV_BLOCK_PASS({
        int c = 0;
        assert(!tm2.for_each([&c](auto) { return ++c == 3; }));  // not found
        assert(c == 2);
    });
}  // namespace
#endif

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
    CAV_PASS(ts1["int"_cs] == 1);
    CAV_PASS(ts1[tag<float>] == 2.0);
    CAV_PASS(ts2[tag<float[10]>][9] == 0);
    CAV_PASS(ts3[tag<int>] == 4);
}  // namespace
#endif

/////////////////////////////////////////////////////////////////////////
///////////////////////////////// TUPLE /////////////////////////////////
/////////////////////////////////////////////////////////////////////////
template <typename V, typename Tag = UNIQUE_TYPE>
struct tuple_elem {
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
