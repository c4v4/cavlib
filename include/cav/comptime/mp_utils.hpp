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

#ifndef CAV_INCLUDE_MP_UTILS_HPP
#define CAV_INCLUDE_MP_UTILS_HPP

#include "../comptime/mp_base.hpp"
#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"

namespace cav {

#ifdef COMP_TESTS
namespace test {
    struct t0 {};

    struct t1 {};

    struct t2 {};

    struct t3 {};

    struct t4 {};

    struct t5 {};
}  // namespace test
#endif


////// change_template //////
/// @brief Given a type which is a template istance, change the template type into another that is
/// compatible with the same template parameters.
template <typename CSetT, template <class...> class Tmpl>
struct change_template;

template <template <class...> class FromTmpl, typename... Ts, template <class...> class ToTmpl>
struct change_template<FromTmpl<Ts...>, ToTmpl> {
    using type = ToTmpl<Ts...>;
};

template <typename CSetT, template <class...> class Tmpl>
using change_template_t = typename change_template<CSetT, Tmpl>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<change_template_t<pack<t1>, inherit>, inherit<t1>>);
    CAV_PASS(eq<change_template_t<inherit<t1, t2>, inherit>, inherit<t1, t2>>);
    CAV_PASS(eq<change_template_t<pack<>, inherit>, inherit<>>);
}  // namespace test
#endif

////// base_template //////
/// @brief Given a type U that might be or inherit from a instantiation of the type-template
/// Tmpl<class..>, retrieve the type of Tmpl instantiation. In case of ambiguity, or if the template
/// is not found, void is returned istead.
template <template <class...> class Tmpl, class U>
struct base_template {
    template <class... Ts>
    static auto test(Tmpl<Ts...> const&) -> Tmpl<Ts...>;
    static void test(...);
    using type                  = TYPEOF(base_template::test(std::declval<U>()));
    static constexpr bool value = !eq<type, void>;
};

template <template <class...> class Tmpl, class U>
using base_template_t = typename base_template<Tmpl, U>::type;

template <template <class...> class Tmpl, typename T>
static constexpr bool is_base_template_v = base_template<Tmpl, T>::value;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<base_template_t<inherit, inherit<t1>>, inherit<t1>>);
    CAV_PASS(eq<base_template_t<inherit, inherit<t1, t2>>, inherit<t1, t2>>);
    CAV_PASS(eq<base_template_t<inherit, inherit<>>, inherit<>>);
}  // namespace test
#endif

////// base_val_template //////
/// @brief Given a type U that might be or inherit from a instantiation of the non-type-template
/// Tmpl<auto...>, retrieve the type of Tmpl instantiation. In case of ambiguity, or if the template
/// is not found, void is returned istead.
template <template <auto...> class Tmpl, typename U>
struct base_val_template {
    template <auto... Ss>
    static Tmpl<Ss...> test(Tmpl<Ss...> const&);
    static void        test(...);
    using type                  = decltype(base_val_template::test(std::declval<U>()));
    static constexpr bool value = !eq<type, void>;
};

template <template <auto...> class Tmpl, typename T>
using base_val_template_t = typename base_val_template<Tmpl, T>::type;

template <template <auto...> class Tmpl, typename T>
static constexpr bool is_base_val_template_v = base_val_template<Tmpl, T>::value;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<base_val_template_t<ct, inherit<ct<1>>>, ct<1>>);
    CAV_PASS(is_base_val_template_v<ct, inherit<ct<1>>>);
}  // namespace test
#endif

/////// nth_type ///////
/// @brief Nth type of a pack, like std::tuple_element<N, std::tuple<Ts...>> but
/// standalone (to avoid the imposition of tuple includes)
#if false && __has_builtin(__type_pack_element)
template <std::size_t N, typename... Ts>
struct nth_type {
    using type = __type_pack_element<N, Ts...>;
};

template <std::size_t N, typename... Ts>
using nth_type_t = __type_pack_element<N, Ts...>;

#else

// Improve readibility of nth_type
namespace detail {
    template <std::size_t>
    struct count {
        constexpr count(auto&& /*unused*/) noexcept {
        }
    };
}  // namespace detail

/// Modified from Kris Jusiak
/// Explanation:
/// 1. Generate the indexes for the the first N types (usual c++20 lambda pattern)
/// 2. Invoke another lambda and consume first N argument declarations with detail:count
/// 3. Take nth argument
/// 4. Forward up to keep the value category
template <std::size_t N>
[[nodiscard]] constexpr decl_auto nth_arg(auto&&... args) noexcept {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decl_auto {
        return [](detail::count<Is>..., auto&& nth, auto&&...) -> decl_auto {
            return FWD(nth);
        }(FWD(args)...);
    }(std::make_index_sequence<N>{});
}

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(nth_arg<0>(0, "1", false) == 0);
    CAV_PASS(nth_arg<1>(0, "1", false)[0] == '1');
    CAV_PASS(!nth_arg<2>(0, "1", false));
}  // namespace test
#endif

template <std::size_t N, typename... Ts>
struct nth_type {
    using type = TYPEOF(nth_arg<N>(wrap<Ts>{}...))::type;
};

template <std::size_t N, typename... Ts>
using nth_type_t = typename nth_type<N, Ts...>::type;

#endif

template <std::size_t N, typename T>
struct nth_type_unwrap;

template <std::size_t N, template <class...> class Tmpl, typename... Ts>
struct nth_type_unwrap<N, Tmpl<Ts...>> {
    using type = nth_type_t<N, Ts...>;
};

template <std::size_t N, typename T>
using nth_type_unwrap_t = typename nth_type_unwrap<N, T>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<nth_type_t<0, t1>, t1>);
    CAV_PASS(eq<nth_type_t<1, t1, t2, t3>, t2>);
    CAV_PASS(eq<nth_type_t<2, t1, t2, t3>, t3>);
    CAV_PASS(eq<nth_type_unwrap_t<0, pack<t1>>, t1>);
    CAV_PASS(eq<nth_type_unwrap_t<1, pack<t1, t2, t3>>, t2>);
    CAV_PASS(eq<nth_type_unwrap_t<2, pack<t1, t2, t3>>, t3>);
}  // namespace test
#endif

////// first_type //////
/// @brief Return the type of the first parameter
/// of a parameter pack.
///
/// @tparam T First parameter type
/// @tparam Args Parameter pack
template <typename...>
struct first_type {
    using type = void;
};

template <class T, class... Ts>
struct first_type<T, Ts...> {
    using type = T;
};
template <class... Ts>
using first_type_t = typename first_type<Ts...>::type;

template <typename>
struct first_type_unwrap {
    using type = void;
};

template <template <class...> class Tmpl, typename... Ts>
struct first_type_unwrap<Tmpl<Ts...>> {
    using type = first_type_t<Ts...>;
};

template <typename T>
using first_type_unwrap_t = typename first_type_unwrap<T>::type;

////// last_type //////
template <class... Ts>
struct last_type {
    using type = nth_type_t<sizeof...(Ts) - 1, Ts...>;
    // typename TYPEOF((std::type_identity<Ts>{}, ...))::type;
};

template <>
struct last_type<> {
    using type = void;
};

template <class... Args>
using last_type_t = typename last_type<Args...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<first_type_t<t1, t2, t3>, t1>);
    CAV_PASS(eq<first_type_t<t1>, t1>);
    CAV_PASS(eq<first_type_t<>, void>);
    CAV_PASS(eq<first_type_unwrap_t<pack<t1, t2, t3>>, t1>);
    CAV_PASS(eq<first_type_unwrap_t<pack<t1>>, t1>);
    CAV_PASS(eq<first_type_unwrap_t<pack<>>, void>);
    CAV_PASS(eq<last_type_t<t1, t2, t3>, t3>);
    CAV_PASS(eq<last_type_t<t1>, t1>);
    CAV_PASS(eq<last_type_t<>, void>);
}  // namespace test
#endif


////// all_equal //////
/// @brief Takes a true-value if all the types of a parameter pack
///  the same, false otherwise.
///
/// @tparam Args parameter pack
template <class... Args>
struct all_equal {
    static constexpr bool value = (eq<first_type_t<Args...>, Args> && ...);
};
template <class... Args>
constexpr bool all_equal_v = all_equal<Args...>::value;

////// all_different //////
/// @brief True if all types of the pack are different.
/// False otherwhise.
template <typename... Ts>
struct all_different : std::true_type {};

template <typename T, typename... Ts>
struct all_different<T, Ts...> {
    static constexpr bool value = !(eq<T, Ts> || ...) && all_different<Ts...>::value;
};

template <typename... Ts>
constexpr bool all_different_v = all_different<Ts...>::value;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(all_equal_v<>);
    CAV_PASS(all_different_v<>);
    CAV_PASS(all_equal_v<t1>);
    CAV_PASS(all_different_v<t1>);
    CAV_PASS(all_different_v<t1, t2>);
    CAV_PASS(!all_equal_v<t1, t2>);
    CAV_PASS(all_equal_v<t1, t1>);
    CAV_PASS(!all_different_v<t1, t1>);
}  // namespace test
#endif

////// has_type_v //////
template <typename T, typename... Ts>
constexpr bool has_type_v = (eq<T, Ts> || ...);

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(!has_type_v<t1>);
    CAV_PASS(has_type_v<t1, t1, t2>);
    CAV_PASS(!has_type_v<t1, t2, t3>);
}  // namespace test
#endif

////// has_template //////
template <template <class...> class Tmpl, typename... Ts>
struct has_template {
    static constexpr bool value = (is_base_template_v<Tmpl, Ts> || ...);
};

template <template <class...> class Tmpl, typename... Ts>
constexpr bool has_template_v = has_template<Tmpl, Ts...>::value;

////// get_template //////
template <template <class...> class Tmpl, typename... Ts>
struct get_template {
    using type = void;
};

template <template <class...> class Tmpl, typename T, typename... Tail>
requires is_base_template_v<Tmpl, T>
struct get_template<Tmpl, T, Tail...> {
    using type = T;
};

template <template <class...> class Tmpl, typename T, typename... Tail>
requires(!is_base_template_v<Tmpl, T>)
struct get_template<Tmpl, T, Tail...> {
    using type = typename get_template<Tmpl, Tail...>::type;
};

template <template <class...> class Tmpl, typename... Ts>
using get_template_t = typename get_template<Tmpl, Ts...>::type;


#ifdef COMP_TESTS
namespace test {
    CAV_PASS(has_template_v<pack, inherit<pack<t1>>, t2>);
    CAV_PASS(has_template_v<pack, inherit<t1>, pack<t2>>);
    CAV_PASS(!has_template_v<pack, inherit<t1>, t2>);
    CAV_PASS(!has_template_v<pack>);
    CAV_PASS(eq<get_template_t<inherit, inherit<>>, inherit<>>);
    CAV_PASS(eq<get_template_t<inherit, inherit<t1>>, inherit<t1>>);
    CAV_PASS(eq<get_template_t<inherit, inherit<t1, t2>>, inherit<t1, t2>>);
}  // namespace test
#endif


////// has_type_unwrap //////
template <typename T, typename PackTmpl>
struct has_type_unwrap : std::false_type {};

template <typename T>
struct has_type_unwrap<T, T> : std::true_type {};

template <typename T, template <class...> class PackTmpl, typename... Ts>
requires(!eq<T, PackTmpl<Ts...>>)
struct has_type_unwrap<T, PackTmpl<Ts...>> {
    static constexpr bool value = (eq<T, Ts> || ...);
};

////// has_template_unwrap //////
template <typename T, typename PackTmpl>
constexpr bool has_type_unwrap_v = has_type_unwrap<T, PackTmpl>::value;

template <template <class...> class Tmpl, typename T>
struct has_template_unwrap {
    static constexpr bool value = is_base_template_v<Tmpl, T>;
};

template <template <class...> class Tmpl, typename... Ts>
struct has_template_unwrap<Tmpl, Tmpl<Ts...>> : std::true_type {};

template <template <class...> class Tmpl, template <class...> class PackTmpl, typename... Ts>
struct has_template_unwrap<Tmpl, PackTmpl<Ts...>> {
    static constexpr bool value = has_template_v<Tmpl, Ts...>;
};
template <template <class...> class Tmpl, typename PackTmpl>
constexpr bool has_template_unwrap_v = has_template_unwrap<Tmpl, PackTmpl>::value;

////// get_template_unwrap //////
template <template <class...> class Tmpl, typename PackTmpl>
struct get_template_unwrap;

template <template <class...> class Tmpl, typename... Ts>
struct get_template_unwrap<Tmpl, Tmpl<Ts...>> {
    using type = Tmpl<Ts...>;
};

template <template <class...> class Tmpl, template <class...> class PackTmpl, typename... Ts>
struct get_template_unwrap<Tmpl, PackTmpl<Ts...>> {
    using type = get_template_t<Tmpl, Ts...>;
};
template <template <class...> class Tmpl, typename PackTmpl>
using get_template_unwrap_t = typename get_template_unwrap<Tmpl, PackTmpl>::type;


#ifdef COMP_TESTS
namespace test {
    CAV_PASS(has_type_unwrap_v<t1, pack<t1>>);
    CAV_PASS(!has_type_unwrap_v<t1, pack<t2>>);
    CAV_PASS(has_template_unwrap_v<pack, inherit<pack<t1>>>);
    CAV_PASS(!has_template_unwrap_v<pack, inherit<t1>>);
    CAV_PASS(eq<get_template_unwrap_t<pack, inherit<pack<t1>>>, pack<t1>>);
    CAV_PASS(!eq<get_template_unwrap_t<pack, inherit<t1>>, pack<t1>>);
}  // namespace test
#endif


////// have_same_types //////
/// @brief Value true if the two packs contains the same types
/// (even if in different order).
template <typename Pack1T, typename Pack2T>
struct have_same_types;

template <template <class...> class TSet1,
          typename... T1s,
          template <class...>
          class TSet2,
          typename... T2s>
struct have_same_types<TSet1<T1s...>, TSet2<T2s...>> {
    static constexpr bool value = (has_type_v<T1s, T2s...> && ...);
};

template <typename Pack1T, typename Pack2T>
constexpr bool have_same_types_v = have_same_types<Pack1T, Pack2T>::value;

////// have_same_template //////
/// @brief Value true if the two types are both instatiation of the same template.
template <typename, typename>
struct have_same_template : std::false_type {};

template <template <class...> class Tmpl, typename... T1s, typename... T2s>
struct have_same_template<Tmpl<T1s...>, Tmpl<T2s...>> : std::true_type {};

template <typename T1, typename T2>
constexpr bool have_same_template_v = have_same_template<T1, T2>::value;

template <typename T1, typename T2>
concept same_template = have_same_template<T1, T2>::value;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(have_same_types_v<pack<t1>, pack<t1>>);
    CAV_PASS(have_same_types_v<pack<t1, t2>, pack<t2, t1>>);
    CAV_PASS(!have_same_types_v<pack<t1, t2>, pack<t1, t3>>);
    CAV_PASS(have_same_template_v<pack<t1>, pack<t2>>);
    CAV_PASS(have_same_template_v<pack<t1, t2>, pack<>>);
    CAV_PASS(!have_same_template_v<pack<t1>, inherit<pack<t1>>>);
}  // namespace test
#endif

///////// PACKS UNION WITH TYPES /////////

////// unique //////
/// @brief Select unique types inside AccPackTmpl.
/// NOTE: single types remains single types to reduce type-name length
///
/// @tparam AccPackTmpl  Accumulator that will be filled
/// @tparam Ts  Types to merge
template <typename AccPackTmpl, typename... Ts>
struct unique {
    CAV_PASS(sizeof...(Ts) == 0);
    using type = AccPackTmpl;
};

template <template <class...> class PackTmpl, typename... AcTs, typename T>
requires(!(eq<T, AcTs> || ...))
struct unique<PackTmpl<AcTs...>, T> {
    using type = PackTmpl<AcTs..., T>;
};

template <template <class...> class PackTmpl, typename... AcTs, typename T, typename... Ts>
requires((eq<T, AcTs> || ...))
struct unique<PackTmpl<AcTs...>, T, Ts...> {
    using type = typename unique<PackTmpl<AcTs...>, Ts...>::type;
};

template <template <class...> class PackTmpl, typename... AcTs, typename T, typename... Ts>
requires(!(eq<T, AcTs> || ...))
struct unique<PackTmpl<AcTs...>, T, Ts...> {
    using type = typename unique<PackTmpl<AcTs..., T>, Ts...>::type;
};

template <typename Acc, typename... Ts>
using unique_t = typename unique<Acc, Ts...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<pack<t1>, unique_t<pack<>, t1>>);
    CAV_PASS(eq<pack<t1>, unique_t<pack<>, t1, t1>>);
    CAV_PASS(eq<pack<t2, t1>, unique_t<pack<>, t2, t1, t1, t2>>);
    CAV_PASS(eq<pack<t1, t2>, unique_t<pack<>, t1, t1, t1, t2>>);
}  // namespace test
#endif

////// fill_unique //////
template <template <class...> class PackTmpl, typename... Ts>
struct fill_unique {
    using type = unique_t<PackTmpl<>, Ts...>;
};
template <template <class...> class PackTmpl, typename... Ts>
using fill_unique_t = typename fill_unique<PackTmpl, Ts...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<fill_unique_t<pack, t1>, pack<t1>>);
    CAV_PASS(eq<fill_unique_t<pack, t1, t1>, pack<t1>>);
    CAV_PASS(eq<fill_unique_t<pack, t2, t1, t1, t2>, pack<t2, t1>>);
    CAV_PASS(eq<fill_unique_t<pack, t1, t1, t1, t2>, pack<t1, t2>>);
}  // namespace test
#endif

namespace detail {
    ////// conditional_wrap //////
    template <template <class...> class PackTmpl, typename T>
    struct conditional_wrap {
        using type = PackTmpl<T>;
    };

    template <template <class...> class PackTmpl, typename... Ts>
    struct conditional_wrap<PackTmpl, PackTmpl<Ts...>> {

        using type = PackTmpl<Ts...>;
    };

    template <template <class...> class PackTmpl, typename T>
    using conditional_wrap_t = typename detail::conditional_wrap<PackTmpl, T>::type;

}  // namespace detail

////// pack_union //////

template <template <class...> class PackTmpl, typename... Ts>
struct pack_union {
    using type = typename pack_union<PackTmpl, detail::conditional_wrap_t<PackTmpl, Ts>...>::type;
};

template <template <class...> class PackTmpl, typename... T1s>
struct pack_union<PackTmpl, PackTmpl<T1s...>> {
    using type = unique_t<PackTmpl<>, T1s...>;
};

template <template <class...> class PackTmpl, typename... T1s, typename... T2s, typename... Ts>
struct pack_union<PackTmpl, PackTmpl<T1s...>, PackTmpl<T2s...>, Ts...> {
    using type = typename pack_union<PackTmpl, unique_t<PackTmpl<>, T1s..., T2s...>, Ts...>::type;
};

template <template <class...> class PackTmpl>
struct pack_union<PackTmpl> {
    using type = PackTmpl<>;
};

template <template <class...> class PackTmpl, typename... Ts>
using pack_union_t = typename pack_union<PackTmpl, Ts...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<pack<t1>, pack_union_t<pack, t1>>);
    CAV_PASS(eq<pack<t1>, pack_union_t<pack, t1, t1>>);
    CAV_PASS(eq<pack<t5, t1>, pack_union_t<pack, t5, t1, t1, t5>>);
    CAV_PASS(eq<pack<t5, t1>, pack_union_t<pack, pack<t5, t1>, t1, t5>>);
    CAV_PASS(eq<pack<t5, t1>, pack_union_t<pack, t5, t1, pack<t5, t1>>>);
    CAV_PASS(eq<pack<t1, t5>, pack_union_t<pack, pack<t1, t1>, pack<t5>>>);
    CAV_PASS(eq<pack<t5, t1>, pack_union_t<pack, pack<t5, t1>, pack<t1>>>);
    CAV_PASS(eq<pack<t1, t5>, pack_union_t<pack, pack<>, pack<t1, t1>, t5>>);
}  // namespace test
#endif


////// fill_pack_excluding //////
template <typename AccT, typename ExT, typename... Ts>
struct fill_pack_excluding;

template <template <class...> class PackTmpl, typename ExT>
struct fill_pack_excluding<PackTmpl<>, ExT, ExT> {
    using type = PackTmpl<ExT>;
};

template <template <class...> class PackTmpl, typename... TPs, typename ExT>
struct fill_pack_excluding<PackTmpl<TPs...>, ExT> {
    using type = PackTmpl<TPs...>;
};

template <template <class...> class PackTmpl,
          typename... PTs,
          typename ExT,
          typename T,
          typename... Ts>
struct fill_pack_excluding<PackTmpl<PTs...>, ExT, T, Ts...> {
    using type = typename fill_pack_excluding<PackTmpl<PTs..., T>, ExT, Ts...>::type;
};

template <template <class...> class PackTmpl, typename... PTs, typename ExT, typename... Ts>
struct fill_pack_excluding<PackTmpl<PTs...>, ExT, ExT, Ts...> {
    using type = typename fill_pack_excluding<PackTmpl<PTs...>, ExT, Ts...>::type;
};

template <typename AccT, typename ExT, typename... Ts>
using fill_pack_excluding_t = typename fill_pack_excluding<AccT, ExT, Ts...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<fill_pack_excluding_t<pack<>, t0, t0>, pack<t0>>);
    CAV_PASS(eq<fill_pack_excluding_t<pack<>, t0, t1>, pack<t1>>);
    CAV_PASS(eq<fill_pack_excluding_t<pack<>, t0, t0, t1, t2, t0, t1>, pack<t1, t2, t1>>);
    CAV_PASS(eq<fill_pack_excluding_t<pack<>, t0, t0, t0, t0>, pack<t0>>);
}  // namespace test
#endif

////// remove_from_pack //////
template <typename AccT, typename ExT>
struct remove_from_pack;

template <template <class...> class PackTmpl, typename... PTs, typename ExT>
struct remove_from_pack<PackTmpl<PTs...>, ExT> {
    using type = fill_pack_excluding_t<PackTmpl<>, ExT, PTs...>;
};

template <typename AccT, typename ExT>
using remove_from_pack_t = typename remove_from_pack<AccT, ExT>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<remove_from_pack_t<pack<t0>, t0>, pack<t0>>);
    CAV_PASS(eq<remove_from_pack_t<pack<t1>, t0>, pack<t1>>);
    CAV_PASS(eq<remove_from_pack_t<pack<t0, t1, t2, t0, t1>, t0>, pack<t1, t2, t1>>);
    CAV_PASS(eq<remove_from_pack_t<pack<t0, t0, t0>, t0>, pack<t0>>);
}  // namespace test
#endif

////// pack_union_excluding //////
template <template <class...> class PackTmpl, typename ExT, typename... Ts>
struct pack_union_excluding {
    using type = remove_from_pack_t<pack_union_t<PackTmpl, Ts...>, ExT>;
};
template <template <class...> class PackTmpl, typename ExT, typename... Ts>
using pack_union_excluding_t = typename pack_union_excluding<PackTmpl, ExT, Ts...>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<pack_union_excluding_t<pack, t0, t0>, pack<t0>>);
    CAV_PASS(eq<pack_union_excluding_t<pack, t0, t1>, pack<t1>>);
    CAV_PASS(eq<pack_union_excluding_t<pack, t0, t0, t1>, pack<t1>>);
    CAV_PASS(eq<pack_union_excluding_t<pack, t0, t0, t1, t2>, pack<t1, t2>>);
    CAV_PASS(eq<pack_union_excluding_t<pack, t0, t0, t1, t2, t1, t2, t2, t2>, pack<t1, t2>>);
}  // namespace test
#endif

////// collapse_if_one //////
template <typename T>
struct collapse_if_one;

template <template <class...> class PackTmpl, typename T>
struct collapse_if_one<PackTmpl<T>> {
    using type = T;
};

template <template <class...> class PackTmpl, typename... Ts>
struct collapse_if_one<PackTmpl<Ts...>> {
    using type = PackTmpl<Ts...>;
};

template <typename T>
using collapse_if_one_t = typename collapse_if_one<T>::type;

#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<collapse_if_one_t<pack<void>>, void>);
    CAV_PASS(eq<collapse_if_one_t<pack<void, t1>>, pack<void, t1>>);
}  // namespace test
#endif

////// pack difference //////

namespace detail {
    template <typename T1, typename, typename>
    struct pack_difference_impl {
        using type = T1;
    };

    template <template <class...> class Tmpl,
              typename... AT,
              typename T,
              typename... T1s,
              typename... T2s>
    struct pack_difference_impl<Tmpl<AT...>, Tmpl<T, T1s...>, Tmpl<T2s...>> {

        using type = if_t<
            (eq_no_cvr<T, T2s> || ...),
            typename pack_difference_impl<Tmpl<AT...>, Tmpl<T1s...>, Tmpl<T2s...>>::type,
            typename pack_difference_impl<Tmpl<AT..., T>, Tmpl<T1s...>, Tmpl<T2s...>>::type>;
    };
}  // namespace detail

template <typename...>
struct pack_difference;

template <template <class...> class Tmpl, typename... T1s, typename... T2s>
struct pack_difference<Tmpl<T1s...>, Tmpl<T2s...>> {
    using type = typename detail::pack_difference_impl<Tmpl<>, Tmpl<T1s...>, Tmpl<T2s...>>::type;
};

template <typename... Ts>
using pack_difference_t = typename pack_difference<Ts...>::type;


#ifdef COMP_TESTS
namespace test {
    CAV_PASS(eq<pack_difference_t<pack<t1, t2, t3, t4>, pack<t1, t4>>, pack<t2, t3>>);
    CAV_PASS(eq<pack_difference_t<pack<t1, t1, t1, t1>, pack<t1, t4>>, pack<>>);
    CAV_PASS(eq<pack_difference_t<pack<t1, t2>, pack<t3, t4>>, pack<t1, t2>>);
    CAV_PASS(eq<pack_difference_t<pack<>, pack<t3, t4>>, pack<>>);
}  // namespace test
#endif

////// not_copy_move_ctor //////
/// @brief Exclude copy and move constructor when using forwarding references.
template <typename T, typename... Ts>
concept not_copy_move_ctor = (sizeof...(Ts) > 1 || !eq_no_cvr<first_type_t<Ts...>, T>);


}  // namespace cav

#endif /* CAV_INCLUDE_MP_UTILS_HPP */
