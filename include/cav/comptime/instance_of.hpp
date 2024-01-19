// Copyright (c) 2023 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef CAV_INCLUDE_INST_OF_HPP
#define CAV_INCLUDE_INST_OF_HPP

#include <type_traits>

#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"

/// @brief Currently, there is no general way to constrain a type to be an instantiation of a given
/// template. All the alternatives I have found so far have some limitations.

namespace cav {

///////////// SOLUTION 1: MACRO /////////////

template <typename T>
constexpr bool not_void_v = !std::is_void_v<no_cvr<T>>;  // Avoid warning on DeMorgan simplification

/// @brief This shouldn't be really a macro.
/// Since C++ is not Circle-lang, there are strong limits on what we can pass as template parameters
/// and what not (unrelated e.g., we cannot pass function templates).
/// In this case, we cannot receive *any* template as an input parameter. We are limited to what
/// variadic packs let us specify, so a template with mixed type and non-type template parameters
/// cannot be matched by a general/universal signature (we do not have a "universal template
/// parameter" that match both type and non-type templates).
/// However, macro are agnostic on whatever is passed to them, so we can create a generic require
/// expression specific for the given template, whatever it is.
/// (Note: variadic macro is used to handle multi-params templates)
///
/// LIMITATIONS:
///   1. It works only with types that can be move-constructed.
///   2. Only works in templated contexts (otherwise can be ill-formed)
///
#define CAV_INST_OF(Tmpl, Type...)                                               \
    (not_void_v<Type> && requires(if_t<not_void_v<Type>, no_cvr<Type>, int> t) { \
        { Tmpl{std::move(t)} } -> cav::eq<no_cvr<Type>>;                         \
    })

///////////// SOLUTION 2: TYPE-TEMPLATE-PARAMETER ONLY /////////////

namespace detail {
    template <class U, template <class...> class Tmpl>
    struct is_inst_of_helper : std::false_type {};

    template <template <class...> class Tmpl, typename... Ts>
    struct is_inst_of_helper<Tmpl<Ts...>, Tmpl> : std::true_type {};
}  // namespace detail

template <class U, template <class...> class Tmpl>
struct is_inst_of : std::false_type {};

/// @brief Clang workaround.
/// Lets suppose we have a template T and an alias Ta, clang recognizes Ta as T, but not T as Ta
/// (while gcc is "bidirectional").
/// When we use "inst_of<Ta<...>, Ta>" the object is considered as an T-object since Ta is an
/// alias of T, and false is returned.
/// To workaround that, we switch the two templates.
template <template <class...> class Tmpl1, template <class...> class Tmpl2, typename... Ts>
requires requires { typename Tmpl2<Ts...>; }  // only if Tmpl2<Ts...> means something
struct is_inst_of<Tmpl1<Ts...>, Tmpl2> : detail::is_inst_of_helper<Tmpl2<Ts...>, Tmpl1> {};

template <class U, template <class...> class Tmpl>
constexpr bool is_inst_of_v = is_inst_of<U, Tmpl>::value;

template <class U, template <class...> class Tmpl>
concept inst_of = is_inst_of_v<no_cvr<U>, Tmpl>;

template <class U, template <class...> class Tmpl>
concept not_inst_of = (!inst_of<U, Tmpl>);

#ifdef CAV_COMP_TESTS
namespace {
    template <typename...>
    struct S {};

    template <auto...>
    struct X {};

    template <auto, typename>
    struct Y {};

    CAV_PASS(inst_of<S<>, S>);
    CAV_PASS(inst_of<S<int>, S>);
    CAV_PASS(inst_of<S<int, float>, S>);
    CAV_PASS(inst_of<S<int, float, double>, S>);
    CAV_PASS(not_inst_of<X<>, S>);

    template <typename T, auto V>
    constexpr bool templated_context() {
        CAV_PASS(CAV_INST_OF(S, S<T>));
        CAV_PASS(CAV_INST_OF(X, X<V>));
        CAV_PASS(CAV_INST_OF(Y, Y<V, T>));
        return true;
    };

    CAV_PASS(templated_context<int, 0>());
}  // namespace
#endif


}  // namespace cav

#endif /* CAV_INCLUDE_INST_OF_HPP */
