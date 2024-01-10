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

#ifndef CAV_INCLUDE_UTILS_CLASSMAP_HPP
#define CAV_INCLUDE_UTILS_CLASSMAP_HPP


#include <type_traits>

#include "../comptime/macros.hpp"
#include "../comptime/mp_utils.hpp"

namespace cav {

template <typename T>
struct key {};

template <typename KeyT, typename ValT>
struct map_elem;

template <typename KeyT, typename ValT>
requires(!std::is_empty_v<ValT>)
struct map_elem<KeyT, ValT> {

    using key_type   = KeyT;
    using value_type = ValT;

    value_type val;

    [[nodiscard]] constexpr value_type& get(key<KeyT> /*k*/) {
        return val;
    }

    [[nodiscard]] constexpr value_type const& get(key<KeyT> /*k*/) const {
        return val;
    }
};

// Could use [[no_unique_address]], but clang-15 crashes, so the empty case is handled explicitly
template <typename KeyT, typename ValT>
requires std::is_empty_v<ValT>
struct map_elem<KeyT, ValT> : ValT {
    using base       = ValT;
    using key_type   = KeyT;
    using value_type = ValT;

    [[nodiscard]] constexpr value_type& get(key<KeyT> /*k*/) {
        return *this;
    }

    [[nodiscard]] constexpr value_type const& get(key<KeyT> /*k*/) const {
        return *this;
    }
};

template <typename...>
struct ClassMap;

template <typename... KeyTs, typename... ValTs>
struct ClassMap<map_elem<KeyTs, ValTs>...> : map_elem<KeyTs, ValTs>... {
    static_assert(all_different_v<KeyTs...>, "Key-type found twice in ClassMap.");
    using keys_type = pack<KeyTs...>;
    using vals_type = pack<ValTs...>;
    using map_elem<KeyTs, ValTs>::get...;
};

template <typename... ClassMapTs>
using class_map_union = pack_union<ClassMap, ClassMapTs...>;
template <typename... ClassMapTs>
using class_map_union_t = typename class_map_union<ClassMapTs...>::type;

#define CAV_SUBTYPES_MAP_UNION(SubT, Pack) \
    class_map_union_t<map_elem<Pack, VOID_IF_ILLEGAL(typename Pack::SubT)>...>

template <typename KeyT, typename... Ts>
[[nodiscard]] constexpr auto& get(ClassMap<Ts...>& map) noexcept {
    static_assert(has_type_unwrap_v<KeyT, typename ClassMap<Ts...>::keys_type>);
    return map.get(key<KeyT>{});
}

template <typename KeyT, typename... Ts>
[[nodiscard]] constexpr auto const& get(ClassMap<Ts...> const& map) noexcept {
    static_assert(has_type_unwrap_v<KeyT, typename ClassMap<Ts...>::keys_type>);
    return map.get(key<KeyT>{});
}

static_assert(get<void>(ClassMap<map_elem<void, int>>{42}) == 42);
static_assert(get<char>(ClassMap<map_elem<char, double>>{0.42}) > 0.1);

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_CLASSMAP_HPP */
