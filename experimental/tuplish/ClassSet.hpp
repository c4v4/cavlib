// Copyright (c) 2022 Francesco Cavaliere
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

#ifndef CAV_INCLUDE_UTILS_CLASSSET_HPP
#define CAV_INCLUDE_UTILS_CLASSSET_HPP


#include <array>
#include <type_traits>

#include "../comptime/instance_of.hpp"  // IWYU pragma: keep
#include "../comptime/macros.hpp"       // IWYU pragma: keep
#include "../comptime/mp_utils.hpp"

namespace cav {

/// @brief ClassSet (trivial implementation, in both senses).
///  Operations supported:
///  - has<typename>(ClassSet) -> bool: true if type is in ClassSet
///  - has<template>(ClassSet) -> bool: true if an istantiation of template is in ClassSet
///  - get<typename>(ClassSet) -> typename: return reference to type in ClassSet
///  - get<template>(ClassSet) -> template<...>: treturn reference to the type that is an
///      istantiation of template within ClassSet. Note: works only if there are no repeated
///      template within the ClassSet.
///  - extract<typename>(ClassSet) -> typename: create a new Set of type typename, extracting
///      the needed data from ClassSet.
template <typename... Ts>
struct ClassSet : Ts... {

    [[nodiscard]] static constexpr int size() noexcept {
        return sizeof...(Ts);
    }

    // Side-effects only, apply lambda to each element
    template <typename LambT>
    void for_each(LambT&& lambda) {
        ((void)lambda(get<Ts>(*this)), ...);
    }

    template <typename LambT>
    void for_each(LambT&& lambda) const {
        ((void)lambda(get<Ts>(*this)), ...);
    }

    // As transform, but the lambda receive all the elements of the tuple as a pack
    auto reduce(auto&& lambda) {
        return lambda(get<Ts>(*this)...);
    }

    auto reduce(auto&& lambda) const {
        return lambda(get<Ts>(*this)...);
    }

    // Apply lambda to each element and return another tuple
    template <typename LambT>
    auto transform(LambT&& lambda) const {
        return reduce([&](Ts const&... elems) { return ClassSet{lambda(elems)...}; });
    }

    // Apply lambda to each element and return an array. Lambda must return always the same type
    auto transform_to_array(auto&& lambda) {
        return reduce([&](auto&&... elem) { return std::array{lambda(elem)...}; });
    }

    auto transform_to_array(auto&& lambda) const {
        return reduce([&](auto&&... elem) { return std::array{lambda(elem)...}; });
    }

    [[nodiscard]] constexpr auto operator<=>(ClassSet<Ts...> const&) const = default;
};

template <typename... Ts>
ClassSet(Ts...) -> ClassSet<Ts...>;

template <typename... Ts>
struct make_class_set {
    using type = fill_unique_t<ClassSet, Ts...>;
};

template <typename... Ts>
using make_class_set_t = typename make_class_set<Ts...>::type;

/// @brief Helper structs for ClassSet functionalities
namespace detail {
    template <template <class...> class Tmpl, typename... Ts>
    struct get_template_checked {
        static_assert(has_template_v<Tmpl, Ts...>,
                      "Error: the requested template is not part of the ClassSet.");
        static_assert(has_template_v<Tmpl, Ts...>,
                      "Error: `get<template>` is ambigous. Multiple instantiations of the given "
                      "template found in the type-set. Note: this function only works for ClassSet "
                      "that contain only one instantiation for template.");
        using type = get_template_t<Tmpl, Ts...>;
    };
    template <template <class...> class Tmpl, typename... Ts>
    using get_template_checked_t = typename get_template_checked<Tmpl, Ts...>::type;

    template <typename To>
    struct extract_helper;

    template <template <class...> class CsetT1, typename... Ts1>
    struct extract_helper<CsetT1<Ts1...>> {
        static_assert(is_base_template_v<ClassSet, CsetT1<Ts1...>>,
                      "Error: Target isn't  ClassSet.");
        using Self = CsetT1<Ts1...>;

        /// @brief Copy from another bigger or equal ClassSet
        template <typename... Ts2>
        static auto build(ClassSet<Ts2...> const& other) -> Self {
            return {get<Ts1>(other)...};
        }

        /// @brief Copy from another bigger or equal ClassSet
        template <typename... Ts2>
        static auto build(ClassSet<Ts2...>&& other) -> Self {
            return {std::move(get<Ts1>(other))...};
        }
    };
}  // namespace detail

template <typename... ClassSetTs>
using class_set_union = pack_union<ClassSet, ClassSetTs...>;
template <typename... ClassSetTs>
using class_set_union_t = typename class_set_union<ClassSetTs...>::type;

/// @brief Test if a ClassSet (or a type) is subset of another.
/// NOTE:
/// - Any type T \in a ClassSet is a subset of that ClassSet.
/// - ClassSet is a subset of itself.
/// - The empty ClassSet is not handled.
template <typename SubTset, typename Tset>
struct is_subset;

template <template <class...> class PackT, typename... Ts1, typename... Ts2>
requires std::derived_from<PackT<Ts2...>, ClassSet<Ts2...>>
struct is_subset<PackT<Ts1...>, PackT<Ts2...>> {
    static constexpr bool value = (has_type_v<Ts1, Ts2...> && ...);
};

template <typename T1, template <class...> class PackT, typename... Ts2>
requires std::derived_from<PackT<Ts2...>, ClassSet<Ts2...>>
struct is_subset<T1, PackT<Ts2...>> {
    static constexpr auto value = has_type_v<T1, Ts2...>;
};

template <typename T1, typename T2>
requires(!is_base_template_v<ClassSet, T2>)
struct is_subset<T1, T2> {
    static constexpr auto value = std::is_same_v<T1, T2>;
};

template <typename SubTset, typename Tset>
constexpr bool is_subset_v = is_subset<SubTset, Tset>::value;

/// @brief Return true if the ClassSet has a type T.
template <typename T, typename... Ts>
[[nodiscard]] constexpr auto has(ClassSet<Ts...> const& /*unused*/) noexcept -> bool {
    return has_type_v<T, Ts...>;
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto has(ClassSet<Ts...> const& /*unused*/) noexcept -> bool {
    return has_template_v<Tmpl, Ts...>;
}

/// @brief Return a reference to the object of type T inside the ClassSet.
template <typename T, typename... Ts>
[[nodiscard]] constexpr auto get(ClassSet<Ts...>& t) noexcept -> T& {
    static_assert(has_type_v<T, Ts...>, "Error: type not found in ClassSet.");
    return static_cast<T&>(t);
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr auto get(ClassSet<Ts...> const& t) noexcept -> T const& {
    static_assert(has_type_v<T, Ts...>, "Error: type not found in ClassSet.");
    return static_cast<T const&>(t);
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto get(ClassSet<Ts...> const& t) noexcept -> auto& {
    using tmpl_inst = detail::get_template_checked_t<Tmpl, Ts...>;
    return get<tmpl_inst>(t);
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto get(ClassSet<Ts...>& t) noexcept -> auto& {
    using tmpl_inst = detail::get_template_checked_t<Tmpl, Ts...>;
    return get<tmpl_inst>(t);
}

template <typename TargetT, typename OriginT>
requires(!std::same_as<TargetT, OriginT>)  //
[[nodiscard]] constexpr auto extract(OriginT&& origin) -> TargetT {
    static_assert(is_base_template_v<ClassSet, OriginT>,
                  "Error: impossible to extract from a non-ClassSet.");
    return detail::extract_helper<TargetT>::build(FWD(origin));
}

/// @brief Transparent functions if ClassSet is collapsed since it contains a single type.
template <typename T1, typename T2>
requires(!is_base_template_v<ClassSet, T2>)  //
[[nodiscard]] constexpr auto has(T2&& /*unused*/) noexcept -> bool {
    return false;
}

template <typename T>
[[nodiscard]] constexpr bool has(T&& /*unused*/) noexcept {
    return true;
}

template <template <class...> class Tmpl, not_inst_of<ClassSet> T>
[[nodiscard]] constexpr bool has(T const& /*unused*/) noexcept {
    return is_base_template_v<Tmpl, T>;
}

template <typename T>
[[nodiscard]] constexpr T const& get(T const& t) noexcept {
    return t;
}

template <typename T>
[[nodiscard]] constexpr T& get(T& t) noexcept {
    return t;
}

template <typename T>
[[nodiscard]] constexpr T&& get(T&& t) noexcept {
    return FWD(t);
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto const& get(Tmpl<Ts...> const& t) noexcept {
    return t;
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto& get(Tmpl<Ts...>& t) noexcept {
    return t;
}

template <template <class...> class Tmpl, typename... Ts>
[[nodiscard]] constexpr auto&& get(Tmpl<Ts...>&& t) noexcept {
    return std::move(t);
}

template <typename T>
[[nodiscard]] constexpr T const& extract(T const& t) noexcept {
    return t;
}

template <typename T>
[[nodiscard]] constexpr T& extract(T& t) noexcept {
    return t;
}

template <typename T>
[[nodiscard]] constexpr T&& extract(T&& t) noexcept {
    return FWD(t);
}

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_CLASSSET_HPP */
