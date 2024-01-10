// Copyright (c) 2023 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CAV_INCLUDE_UTILS_CLASSCACHE_HPP
#define CAV_INCLUDE_UTILS_CLASSCACHE_HPP

#include <concepts>

#include "../tuplish/ClassMap.hpp"

namespace cav {


/// @brief ClassMap inspired data structure.
/// ClassCache is a semi-compile-time cache that uses types as keys.
/// The interface provide 3 main operations:
///
/// 1. has_value<KeyType>(cache_obj)   => true if a value is stored in the cache.
/// 2. get<KeyType>(cache_obj)         => get the reference of the value stored in the cache
///                                       (can be uninitialized).
/// 3. get<KeyType>(cache_obj, lambda) => if the value is not found, it computes it using lambda,
///                                       otherwise it simply returns the cached value.
template <typename...>
struct ClassCache;


#ifndef CAV_CLASS_CACHE_VERSION
#define CAV_CLASS_CACHE_VERSION 2
#endif
#if CAV_CLASS_CACHE_VERSION > 4
#error "CAV_CLASS_CACHE_VERSION must be between 0 and 4."
#endif

/// @brief Tried some different implementation with 8 cached values to test which one produces the
/// best assembly (https://godbolt.org/z/eKKETxT3b).
/// The "simulation" is done using random() as source of data and some "concatenation" of REF with
/// operations similar to the real ones. Still, the real cases can become quite more hairy than this
/// one, while simple variants, like the CCAV, should be much simpler.
///
/// MANUAL BEST CASE NO-CACHE: asm lines: 378; cond jumps: 7;  rand calls: 6;  stack occup: 216
/// CAV_CLASS_CACHE_VERSION 1: asm lines: 608; cond jumps: 9;  rand calls: 10; stack occup: 232
/// CAV_CLASS_CACHE_VERSION 2: asm lines: 406; cond jumps: 8;  rand calls: 6;  stack occup: 288 <==
/// CAV_CLASS_CACHE_VERSION 3: asm lines: 608; cond jumps: 9;  rand calls: 10; stack occup: 232
/// CAV_CLASS_CACHE_VERSION 4: asm lines: 579; cond jumps: 10; rand calls: 10; stack occup: 224

#if CAV_CLASS_CACHE_VERSION == 1

/// @brief Version using two ClassMaps: one for values and one for flags
template <typename... KeyTs, typename... ValTs>
struct ClassCache<map_elem<KeyTs, ValTs>...> {
    using val_map_t  = ClassMap<map_elem<KeyTs, ValTs>...>;
    using flag_map_t = ClassMap<map_elem<KeyTs, bool>...>;

    val_map_t  vals;
    flag_map_t has_vals = {};  // false init
};

#elif CAV_CLASS_CACHE_VERSION == 2

template <class T>
struct flagged_val {
    [[no_unique_address]] T val;
    bool                    has_val;
};

/// @brief Version using one ClassMaps of "optional"-like aggregates
template <typename... KeyTs, typename... ValTs>
struct ClassCache<map_elem<KeyTs, ValTs>...> : ClassMap<map_elem<KeyTs, flagged_val<ValTs>>...> {};

#elif CAV_CLASS_CACHE_VERSION == 3

/// @brief Version using one ClassMaps for values + 1 array for flags
template <typename... KeyTs, typename... ValTs>
struct ClassCache<map_elem<KeyTs, ValTs>...> : ClassMap<map_elem<KeyTs, ValTs>...> {

    static constexpr ClassMap<map_elem<KeyTs, int>...> num_map =
        []<size_t... Is>(std::index_sequence<Is...>) {
            return ClassMap<map_elem<KeyTs, int>...>{Is...};
        }(std::index_sequence_for<KeyTs...>{});

    std::array<bool, sizeof...(KeyTs)> flags{};
};

#elif CAV_CLASS_CACHE_VERSION == 4

/// @brief Version using one ClassMaps for values + 1 packed array for flags (bit-array)
template <typename... KeyTs, typename... ValTs>
struct ClassCache<map_elem<KeyTs, ValTs>...> : ClassMap<map_elem<KeyTs, ValTs>...> {
    static constexpr ClassMap<map_elem<KeyTs, int>...> num_map =
        []<size_t... Is>(std::index_sequence<Is...>) {
            return ClassMap<map_elem<KeyTs, int>...>{Is...};
        }(std::index_sequence_for<KeyTs...>{});

    static constexpr size_t                      sz = sizeof...(KeyTs);
    std::array<uint8_t, (sz / 8) + (sz % 8 > 0)> flags{};  // use bitset instead!
};

#endif

template <class KeyT, class... ElemTs>
[[nodiscard]] static constexpr bool has_value(ClassCache<ElemTs...> const& map) noexcept {
#if CAV_CLASS_CACHE_VERSION == 1
    return map.has_vals.get(key<KeyT>{});

#elif CAV_CLASS_CACHE_VERSION == 2
    return map.get(key<KeyT>{}).has_val;

#elif CAV_CLASS_CACHE_VERSION == 3
    return map.flags[get<KeyT>(ClassCache<ElemTs...>::num_map)];

#elif CAV_CLASS_CACHE_VERSION == 4
    constexpr size_t idx = get<KeyT>(ClassCache<ElemTs...>::num_map);
    return (map.flags[idx / 8U] & (1U << idx % 8U)) != 0;
#endif
}

template <class KeyT, class... ElemTs>
[[nodiscard]] static constexpr auto& get(ClassCache<ElemTs...>& map) noexcept {

#if CAV_CLASS_CACHE_VERSION == 1
    return map.vals.get(key<KeyT>{});

#elif CAV_CLASS_CACHE_VERSION == 2
    return map.get(key<KeyT>{}).val;

#elif CAV_CLASS_CACHE_VERSION >= 3
    return map.get(key<KeyT>{});
#endif
}

template <class KeyT, class... ElemTs>
[[nodiscard]] static constexpr auto& get(ClassCache<ElemTs...>& map, auto&& fallback) noexcept {

    if (!has_value<KeyT>(map))
        set<KeyT>(map, FWD(fallback)());
    else if constexpr (std::equality_comparable<TYPEOF(get<KeyT>(map))>)
        assert(get<KeyT>(map) == FWD(fallback)());

    return get<KeyT>(map);
}

template <class KeyT, class... ElemTs>
[[nodiscard]] static constexpr auto const& get(ClassCache<ElemTs...> const& map) noexcept {

    assert((has_value<KeyT>(map)));

#if CAV_CLASS_CACHE_VERSION == 1
    return map.vals.get(key<KeyT>{});

#elif CAV_CLASS_CACHE_VERSION == 2
    return map.get(key<KeyT>{}).val;

#elif CAV_CLASS_CACHE_VERSION >= 3
    return map.get(key<KeyT>{});
#endif
}

template <class CacheT, class... KT>
using cached_type = TYPEOF(get<KT...>(std::declval<CacheT>()));

template <class KeyT, class ValT, class... ElemTs>
static constexpr void set(ClassCache<ElemTs...>& map, ValT&& val) noexcept {
    static_assert(std::same_as<no_cvr<ValT>, cached_type<ClassCache<ElemTs...>, KeyT>>);
    get<KeyT>(map) = FWD(val);

#if CAV_CLASS_CACHE_VERSION == 1
    map.has_vals.get(key<KeyT>{}) = true;

#elif CAV_CLASS_CACHE_VERSION == 2
    map.get(key<KeyT>{}).has_val = true;

#elif CAV_CLASS_CACHE_VERSION == 3
    map.flags[get<KeyT>(ClassCache<ElemTs...>::num_map)] = true;

#elif CAV_CLASS_CACHE_VERSION == 4
    constexpr size_t idx = get<KeyT>(ClassCache<ElemTs...>::num_map);
    map.flags[idx / 8U] |= (1U << idx % 8U);
#endif
}

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_CLASSCACHE_HPP */
