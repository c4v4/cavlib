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

#ifndef CAV_TUPLISH_TYPE_MAP_HPP
#define CAV_TUPLISH_TYPE_MAP_HPP

#include "../comptime/call_utils.hpp"
#include "../comptime/mp_base.hpp"
#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/type_name.hpp"

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
    using key_t   = K;
    using value_t = V;

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
};

template <typename... Ts>
struct type_map : Ts... {
    static_assert(!(eq<typename Ts::value_t, void> || ...));

    template <typename T>
    static constexpr auto ts_name = type_name<typename T::key_t>::name;

    using Ts::operator[]...;

    [[nodiscard]] static consteval size_t size() {
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

    template <typename T>
    [[nodiscard]] static consteval bool has(tag_type<T> /*k*/) {
        return has_type_v<T, Ts...>;
    }

    template <value_wrap<std::size_t> K>
    [[nodiscard]] static consteval bool has(ct<K> /*k*/) {
        return K.value < size();
    }

    template <std::size_t N, value_wrap<StaticStr<N>> K>
    [[nodiscard]] static consteval bool has(ct<K> /*k*/) {
        constexpr auto   tname        = static_cast<StaticStr<N>>(K);
        constexpr size_t nexact_match = count_trues(tname == ts_name<Ts>...);
        return nexact_match == 1 ||
               (nexact_match < 1 && count_trues(ts_name<Ts>.starts_with(tname)...) == 1);
    }

    template <value_wrap<std::size_t> K>
    [[nodiscard]] static consteval size_t get_idx(ct<K> /*k*/) {
        static_assert(size() > 0, "Requesting element of empty type_map");
        static_assert(K < size(), "Index out of bounds");
        return K.value;
    }

    template <std::size_t N, value_wrap<StaticStr<N>> K>
    [[nodiscard]] static consteval size_t get_idx(ct<K> /*k*/) {
        constexpr auto tname = static_cast<StaticStr<N>>(K);
        if constexpr (count_trues(tname == ts_name<Ts>...) == 1)
            return idx_of_true(tname == ts_name<Ts>...);
        else
            return idx_of_true(ts_name<Ts>.starts_with(tname)...);
    }

    template <auto K>
    requires(has(ct_v<K>))
    [[nodiscard]] constexpr decl_auto operator[](ct<K> k) && {
        return std::move(nth_type_t<get_idx(k), Ts...>::value);
    }

    template <auto K>
    requires(has(ct_v<K>))
    [[nodiscard]] constexpr auto const& operator[](ct<K> k) const& {
        return nth_type_t<get_idx(k), Ts...>::value;
    }

    template <auto K>
    requires(has(ct_v<K>))
    [[nodiscard]] constexpr auto& operator[](ct<K> k) & {
        return nth_type_t<get_idx(k), Ts...>::value;
    }
};

template <typename, typename>
struct make_tmap_from_lists;

template <typename... Ks, typename... Vs>
struct make_tmap_from_lists<pack<Ks...>, pack<Vs...>> {
    static_assert(sizeof...(Ks) == sizeof...(Vs), "Number of keys and values must be the same");
    using type = type_map<map_elem<Ks, Vs>...>;
};

template <typename Ks, typename Vs>
using make_tmap_from_lists_t = typename make_tmap_from_lists<Ks, Vs>::type;


#ifdef CAV_COMP_TESTS
namespace {
    constexpr inline auto tm1 = type_map<map_elem<int, int>, map_elem<ct<2>, float>>{1, 2.0};
    constexpr inline auto tm2 = type_map<map_elem<ct<5>, int>, map_elem<float, float[10]>>{3, {}};
    constexpr inline auto tm3 = type_map<map_elem<int, int>>{4};
    constexpr inline auto tm4 = type_map<>{};
    constexpr inline auto tm5 = make_tmap_from_lists_t<pack<pack<int>, pack<>, pack<float>>,
                                                       pack<int, int16_t, float>>{1, {}, 2.0};

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

    CAV_PASS(tm5["cav::pack<i"_cs] == 1);
    CAV_PASS(tm5["cav::pack<in"_cs] == 1);
    CAV_PASS(tm5["cav::pack<int"_cs] == 1);
    CAV_PASS(tm5["cav::pack<int>"_cs] == 1);

    CAV_FAIL(eq<decltype(tm5["cav::pack<>"_cs]), void>);

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
}  // namespace cav

#endif /* CAV_TUPLISH_TYPE_MAP_HPP */
