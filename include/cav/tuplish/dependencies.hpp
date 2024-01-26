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

#ifndef CAV_TUPLISH_DEPENDENCIES_HPP
#define CAV_TUPLISH_DEPENDENCIES_HPP

#include "../tuplish/type_set.hpp"
#include "../comptime/mp_utils.hpp"

namespace cav {

/// @brief Simple dependency resolution system.
/// There are two approach:
/// 1. Lazy: list the dependencies in the deps member type using pack, then do the flattening union.
/// 2. Tidy: list the resolved dependencies, such that the final type has a recursion that is always
/// of depth 1.
///
/// Public metafunctions exported:
/// - resolve_deps: resolve the dependencies of a type.
/// - resolve_deps_t: alias for resolve_deps<T>::type.
/// - deps_t: intended to be used to list the dependencies of a type
/// - make_dependency_set_t: create tuple-like object with the resolved dependencies


/////// resolve_deps ///////

template <typename... Ts>
struct resolve_deps;

namespace detail {
    // See if T::deps exists, otherwise return empty pack
    // (Specialized version of the more general CAV_FALLBK_TYPE in macros.hpp)
    template <typename T>
    struct extract_deps {
        using type = pack<>;
    };

    template <typename T>
    requires REQUIRES_TYPE(T::deps)
    struct extract_deps<T> {
        using type = typename resolve_deps<typename T::deps>::type;
    };

    template <typename T>
    using extract_deps_t = typename extract_deps<T>::type;
}  // namespace detail

/// @brief Resolve the dependencies of a type.
template <typename... Ts>
struct resolve_deps : flat_pack_union<detail::extract_deps_t<Ts>..., Ts...> {};

template <typename... Ts>
struct resolve_deps<pack<Ts...>> : resolve_deps<Ts...> {};

template <typename... Ts>
using resolve_deps_t = typename resolve_deps<Ts...>::type;

/// @brief Intended to be used to list the dependencies of a type (even though using pack is also
/// ok). Using deps_t resolve dependenciens type by type avoiding the single gigantic resolution
/// that might happen using simply pack. Indeed, has the same definition of resolve_deps_t.
template <typename... Ts>
using deps_t = typename resolve_deps<Ts...>::type;

template <typename... Ts>
using make_dependency_set_t = tmpl_cast_t<type_set, typename resolve_deps<Ts...>::type>;

template <typename T, typename U>
constexpr bool is_dep_v = has_type_unwrap_v<T, typename U::deps>;

#ifdef CAV_COMP_TESTS
namespace {

#define CAV_TEST_TYPE_WITH_DEPS(X, ...)   \
    struct X {                            \
        using deps = deps_t<__VA_ARGS__>; \
    };

    struct NODEPS {};

    CAV_TEST_TYPE_WITH_DEPS(A);
    CAV_TEST_TYPE_WITH_DEPS(B, A)
    CAV_TEST_TYPE_WITH_DEPS(C, A)
    CAV_TEST_TYPE_WITH_DEPS(D, C, A)
    CAV_TEST_TYPE_WITH_DEPS(E, C, A)
    CAV_TEST_TYPE_WITH_DEPS(F, C, E, B)
    CAV_TEST_TYPE_WITH_DEPS(G, F, D)

    CAV_PASS(cav::eq<resolve_deps_t<A>, pack<A>>);
    CAV_PASS(cav::eq<resolve_deps_t<NODEPS>, pack<NODEPS>>);
    CAV_PASS(cav::eq<resolve_deps_t<B>, pack<A, B>>);
    CAV_PASS(cav::eq<resolve_deps_t<C>, pack<A, C>>);
    CAV_PASS(cav::eq<resolve_deps_t<D>, pack<A, C, D>>);
    CAV_PASS(cav::eq<resolve_deps_t<E>, pack<A, C, E>>);
    CAV_PASS(cav::eq<resolve_deps_t<F>, pack<A, C, E, B, F>>);
    CAV_PASS(cav::eq<resolve_deps_t<G>, pack<A, C, E, B, F, D, G>>);

    CAV_PASS(cav::eq<resolve_deps_t<>, pack<>>);
    CAV_PASS(cav::eq<resolve_deps_t<A, F>, pack<A, C, E, B, F>>);
    CAV_PASS(cav::eq<resolve_deps_t<F::deps>, pack<A, C, E, B>>);
    CAV_PASS(cav::eq<resolve_deps_t<A, F, D>, pack<A, C, E, B, F, D>>);

    // Expantion
    // ^  F               D
    // | ~C  E      B    ~C ~A
    // | ~A  C ~A  ~A    ~A
    // |     A
    // Result: A  ~A C ~A ~A ~A  ~C E B ~C ~A  F D  =>  A C E B F D
}  // namespace
#endif
}  // namespace cav

#endif /* CAV_TUPLISH_DEPENDENCIES_HPP */
