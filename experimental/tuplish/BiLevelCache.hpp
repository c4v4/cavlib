// Copyright (c) 2023 Francesco Cavaliere
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

#ifndef CAV_INCLUDE_UTILS_BILEVELCACHE_HPP
#define CAV_INCLUDE_UTILS_BILEVELCACHE_HPP

#include "../tuplish/ClassCache.hpp"

namespace cav {

template <typename, typename, template <class, class> class>
struct map_to_type_cache;

/// @brief Simple cache wrapper using the cartesian product of 2 key-sets as key.
/// Value-types are provided generically as a template that, given the two keys, returns the stored
/// type. In this way the interface remains general and we can express cleanly the common case where
/// all fields have the same value or the value can be retrieved as a "meta-function" of the keys.
template <typename Keys1T, typename Keys2T, template <class, class> class GetValTmpl>
struct BiLevelCache : map_to_type_cache<Keys1T, Keys2T, GetValTmpl>::type {};

template <typename... Key1Ts, typename... Key2Ts, template <class, class> class GetValTmpl>
struct map_to_type_cache<pack<Key1Ts...>, pack<Key2Ts...>, GetValTmpl> {
    template <class T, template <class, class> class VTmpl>
    using sub_cache_t = ClassCache<map_elem<pack<T, Key2Ts>, typename VTmpl<T, Key2Ts>::type>...>;
    using type        = pack_union_t<ClassCache, sub_cache_t<Key1Ts, GetValTmpl>...>;
};

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
[[nodiscard]] static constexpr bool has_value(
    BiLevelCache<KsT1, KsT2, VGetTmpl> const& map) noexcept {

    return has_value<pack<KT1, KT2>>(map);
}

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
[[nodiscard]] static constexpr bool& has_value(BiLevelCache<KsT1, KsT2, VGetTmpl>& map) noexcept {

    return has_value<pack<KT1, KT2>>(map);
}

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
[[nodiscard]] static constexpr auto& get(BiLevelCache<KsT1, KsT2, VGetTmpl>& map) noexcept {

    return get<pack<KT1, KT2>>(map);
}

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
[[nodiscard]] static constexpr auto& get(BiLevelCache<KsT1, KsT2, VGetTmpl>& map,
                                         auto&&                              fallback) noexcept {

    return get<pack<KT1, KT2>>(map, FWD(fallback));
}

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
[[nodiscard]] static constexpr auto const& get(
    BiLevelCache<KsT1, KsT2, VGetTmpl> const& map) noexcept {

    assert((has_value<KT1, KT2>(map)));
    return get<pack<KT1, KT2>>(map);
}

template <class KT1, class KT2, class KsT1, class KsT2, template <class, class> class VGetTmpl>
static constexpr void set(BiLevelCache<KsT1, KsT2, VGetTmpl> const& map, auto const& val) noexcept {
    static_assert(std::same_as<TYPEOF(val), TYPEOF((get<KT1, KT2>(map)))>);

    has_value<KT1, KT2>(map) = true;
    get<KT1, KT2>(map)       = val;
}

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_BILEVELCACHE_HPP */
