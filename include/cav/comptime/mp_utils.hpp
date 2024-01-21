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

#include <type_traits>

#include "../comptime/mp_base.hpp"
#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"

namespace cav {

#ifdef CAV_COMP_TESTS
namespace {
    struct t0 {};

    struct t1 {};

    struct t2 {};

    struct t3 {};

    struct t4 {};

    struct t5 {};
}  // namespace
#endif


////// tmpl_cast //////
/// @brief Given a type which is a template istance, change the template type into another that is
/// compatible with the same template parameters.
template <template <class...> class Tmpl, typename T>
struct tmpl_cast;

template <template <class...> class FromTmpl, typename... Ts, template <class...> class ToTmpl>
struct tmpl_cast<ToTmpl, FromTmpl<Ts...>> {
    using type = ToTmpl<Ts...>;
};

template <template <class...> class Tmpl, typename T>
using tmpl_cast_t = typename tmpl_cast<Tmpl, T>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<tmpl_cast_t<inherit, pack<t1>>, inherit<t1>>);
    CAV_PASS(eq<tmpl_cast_t<inherit, inherit<t1, t2>>, inherit<t1, t2>>);
    CAV_PASS(eq<tmpl_cast_t<inherit, pack<>>, inherit<>>);
}  // namespace
#endif

////// base_tmpl //////
/// @brief Given a type U that might be or inherit from a instantiation of the type-template
/// Tmpl<class..>, retrieve the type of Tmpl instantiation. In case of ambiguity, or if the template
/// is not found, void is returned istead.
template <template <class...> class Tmpl, class U>
struct base_tmpl {
    template <class... Ts>
    static auto test(Tmpl<Ts...> const&) -> Tmpl<Ts...>;
    static void test(...);
    using type = TYPEOF(base_tmpl::test(std::declval<U>()));

    static constexpr bool value = !eq<type, void>;
};

template <template <class...> class Tmpl, class U>
using base_tmpl_t = typename base_tmpl<Tmpl, U>::type;

template <template <class...> class Tmpl, typename T>
static constexpr bool is_base_tmpl_v = base_tmpl<Tmpl, T>::value;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<base_tmpl_t<inherit, inherit<t1>>, inherit<t1>>);
    CAV_PASS(eq<base_tmpl_t<inherit, inherit<t1, t2>>, inherit<t1, t2>>);
    CAV_PASS(eq<base_tmpl_t<inherit, inherit<>>, inherit<>>);
}  // namespace
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

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<base_val_template_t<ct, inherit<ct<1>>>, ct<1>>);
    CAV_PASS(is_base_val_template_v<ct, inherit<ct<1>>>);
}  // namespace
#endif

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

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(nth_arg<0>(0, "1", false) == 0);
    CAV_PASS(nth_arg<1>(0, "1", false)[0] == '1');
    CAV_PASS(!nth_arg<2>(0, "1", false));
}  // namespace
#endif

/////// nth_type ///////
/// @brief Nth type of a pack, like std::tuple_element<N, std::tuple<Ts...>> but
/// standalone (to avoid the imposition of tuple includes)
#if __has_builtin(__type_pack_element)
template <std::size_t N, typename... Ts>
struct nth_type {
    using type = __type_pack_element<N, Ts...>;
};

template <std::size_t N, typename... Ts>
using nth_type_t = __type_pack_element<N, Ts...>;

#else

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

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<nth_type_t<0, t1>, t1>);
    CAV_PASS(eq<nth_type_t<1, t1, t2, t3>, t2>);
    CAV_PASS(eq<nth_type_t<2, t1, t2, t3>, t3>);
    CAV_PASS(eq<nth_type_unwrap_t<0, pack<t1>>, t1>);
    CAV_PASS(eq<nth_type_unwrap_t<1, pack<t1, t2, t3>>, t2>);
    CAV_PASS(eq<nth_type_unwrap_t<2, pack<t1, t2, t3>>, t3>);
}  // namespace
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
#if __has_builtin(__type_pack_element)
    using type = nth_type_t<sizeof...(Ts) - 1, Ts...>;
#else
    using type = typename TYPEOF((std::type_identity<Ts>{}, ...))::type;
#endif
};

template <>
struct last_type<> {
    using type = void;
};

template <class... Args>
using last_type_t = typename last_type<Args...>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<first_type_t<t1, t2, t3>, t1>);
    CAV_PASS(eq<first_type_t<t1>, t1>);
    CAV_PASS(eq<first_type_t<>, void>);
    CAV_PASS(eq<first_type_unwrap_t<pack<t1, t2, t3>>, t1>);
    CAV_PASS(eq<first_type_unwrap_t<pack<t1>>, t1>);
    CAV_PASS(eq<first_type_unwrap_t<pack<>>, void>);
    CAV_PASS(eq<last_type_t<t1, t2, t3>, t3>);
    CAV_PASS(eq<last_type_t<t1>, t1>);
    CAV_PASS(eq<last_type_t<>, void>);
}  // namespace
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

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(all_equal_v<>);
    CAV_PASS(all_different_v<>);
    CAV_PASS(all_equal_v<t1>);
    CAV_PASS(all_different_v<t1>);
    CAV_PASS(all_different_v<t1, t2>);
    CAV_PASS(!all_equal_v<t1, t2>);
    CAV_PASS(all_equal_v<t1, t1>);
    CAV_PASS(!all_different_v<t1, t1>);
}  // namespace
#endif

////// has_type_v //////
template <typename T, typename... Ts>
constexpr bool has_type_v = (eq<T, Ts> || ...);

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(!has_type_v<t1>);
    CAV_PASS(has_type_v<t1, t1, t2>);
    CAV_PASS(!has_type_v<t1, t2, t3>);
}  // namespace
#endif

////// find_tmpl //////
template <template <class...> class Tmpl, typename... Ts>
struct find_tmpl {
    static_assert(sizeof...(Ts) == 0);  // Check instantiation happened correctly
    static constexpr bool value = false;
    using type                  = void;
};

template <template <class...> class Tmpl, typename T, typename... Tail>
requires is_base_tmpl_v<Tmpl, T>
struct find_tmpl<Tmpl, T, Tail...> {
    static constexpr bool value = true;
    using type                  = base_tmpl_t<Tmpl, T>;
};

template <template <class...> class Tmpl, typename T, typename... Tail>
requires(!is_base_tmpl_v<Tmpl, T>)
struct find_tmpl<Tmpl, T, Tail...> : find_tmpl<Tmpl, Tail...> {};

template <template <class...> class Tmpl, typename... Ts>
using find_tmpl_t = typename find_tmpl<Tmpl, Ts...>::type;

template <template <class...> class Tmpl, typename... Ts>
constexpr bool find_tmpl_v = find_tmpl<Tmpl, Ts...>::value;


#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(find_tmpl_v<pack, inherit<pack<t1>>, t2>);
    CAV_PASS(find_tmpl_v<pack, inherit<t1>, pack<t2>>);
    CAV_PASS(!find_tmpl_v<pack, inherit<t1>, t2>);
    CAV_PASS(!find_tmpl_v<pack>);
    CAV_PASS(eq<find_tmpl_t<inherit, inherit<>>, inherit<>>);
    CAV_PASS(eq<find_tmpl_t<inherit, inherit<t1>>, inherit<t1>>);
    CAV_PASS(eq<find_tmpl_t<inherit, inherit<t1, t2>>, inherit<t1, t2>>);
}  // namespace
#endif


////// has_type_unwrap //////
template <typename T, typename PackT>
struct has_type_unwrap : std::false_type {};

template <typename T, template <class...> class Tmpl, typename... Ts>
struct has_type_unwrap<T, Tmpl<Ts...>> {
    static constexpr bool value = (eq<T, Ts> || ...);
};

template <typename T, typename PackT>
constexpr bool has_type_unwrap_v = has_type_unwrap<T, PackT>::value;

////// find_tmpl_unwrap //////
template <template <class...> class Tmpl, typename T>
struct find_tmpl_unwrap;

template <template <class...> class Tmpl1, template <class...> class Tmpl2, typename... Ts>
struct find_tmpl_unwrap<Tmpl1, Tmpl2<Ts...>> : find_tmpl<Tmpl1, Ts...> {};

template <template <class...> class Tmpl, typename PackT>
constexpr bool find_tmpl_unwrap_v = find_tmpl_unwrap<Tmpl, PackT>::value;

template <template <class...> class Tmpl, typename PackT>
using find_tmpl_unwrap_t = typename find_tmpl_unwrap<Tmpl, PackT>::type;


#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(has_type_unwrap_v<t1, pack<t1>>);
    CAV_PASS(!has_type_unwrap_v<t1, pack<t2>>);
    CAV_PASS(find_tmpl_unwrap_v<pack, inherit<pack<t1>>>);
    CAV_PASS(!find_tmpl_unwrap_v<pack, inherit<t1>>);
    CAV_PASS(eq<find_tmpl_unwrap_t<pack, inherit<pack<t1>>>, pack<t1>>);
    CAV_PASS(!eq<find_tmpl_unwrap_t<pack, inherit<t1>>, pack<t1>>);
}  // namespace
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

////// are_tmpl_eq //////
/// @brief Value true if the two types are both instatiation of the same template.
template <typename, typename>
struct are_tmpl_eq : std::false_type {};

template <template <class...> class Tmpl, typename... T1s, typename... T2s>
struct are_tmpl_eq<Tmpl<T1s...>, Tmpl<T2s...>> : std::true_type {};

template <typename T1, typename T2>
constexpr bool are_tmpl_v_eq = are_tmpl_eq<T1, T2>::value;

template <typename T1, typename T2>
concept eq_tmpl = are_tmpl_eq<T1, T2>::value;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(have_same_types_v<pack<t1>, pack<t1>>);
    CAV_PASS(have_same_types_v<pack<t1, t2>, pack<t2, t1>>);
    CAV_PASS(!have_same_types_v<pack<t1, t2>, pack<t1, t3>>);
    CAV_PASS(are_tmpl_v_eq<pack<t1>, pack<t2>>);
    CAV_PASS(are_tmpl_v_eq<pack<t1, t2>, pack<>>);
    CAV_PASS(!are_tmpl_v_eq<pack<t1>, inherit<pack<t1>>>);
}  // namespace
#endif

///////// PACKS UNION WITH TYPES /////////

namespace detail {
    template <typename AccPackT, typename... Ts>
    struct unique_impl {
        static_assert(sizeof...(Ts) == 0);
        using type = AccPackT;
    };

    template <typename... AcTs, typename T, typename... Ts>
    requires((eq<T, AcTs> || ...))
    struct unique_impl<pack<AcTs...>, T, Ts...> : unique_impl<pack<AcTs...>, Ts...> {};

    template <typename... AcTs, typename T, typename... Ts>
    requires(!(eq<T, AcTs> || ...))
    struct unique_impl<pack<AcTs...>, T, Ts...> : unique_impl<pack<AcTs..., T>, Ts...> {};
}  // namespace detail

////// unique //////
/// @brief Select unique types inside AccPackT.
/// NOTE: single types remains single types to reduce type-name length
///
/// @tparam AccPackT  Accumulator that will be filled
/// @tparam Ts        Types to merge
template <typename... Ts>
struct unique : detail::unique_impl<pack<>, Ts...> {};

template <typename... Ts>
using unique_t = typename detail::unique_impl<pack<>, Ts...>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<pack<t1>, unique_t<t1>>);
    CAV_PASS(eq<pack<t1>, unique_t<t1, t1>>);
    CAV_PASS(eq<pack<t2, t1>, unique_t<t2, t1, t1, t2>>);
    CAV_PASS(eq<pack<t1, t2>, unique_t<t1, t1, t1, t2>>);
}  // namespace
#endif

namespace detail {
    ////// conditional_wrap //////
    template <typename T>
    struct conditional_wrap {
        using type = pack<T>;
    };

    template <typename... Ts>
    struct conditional_wrap<pack<Ts...>> {
        using type = pack<Ts...>;
    };
}  // namespace detail

////// flat_pack_union //////

template <typename... Ts>
struct flat_pack_union : flat_pack_union<typename detail::conditional_wrap<Ts>::type...> {};

template <typename... T1s>
struct flat_pack_union<pack<T1s...>> : unique<T1s...> {};

template <typename... T1s, typename... T2s, typename... Ts>
struct flat_pack_union<pack<T1s...>, pack<T2s...>, Ts...>
    : flat_pack_union<pack<T1s..., T2s...>, Ts...> {};

template <>
struct flat_pack_union<> {
    using type = pack<>;
};

template <typename... Ts>
using flat_pack_union_t = typename flat_pack_union<Ts...>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<pack<t1>, flat_pack_union_t<t1>>);
    CAV_PASS(eq<pack<t1>, flat_pack_union_t<t1, t1>>);
    CAV_PASS(eq<pack<t5, t1>, flat_pack_union_t<t5, t1, t1, t5>>);
    CAV_PASS(eq<pack<t5, t1>, flat_pack_union_t<pack<t5, t1>, t1, t5>>);
    CAV_PASS(eq<pack<t5, t1>, flat_pack_union_t<t5, t1, pack<t5, t1>>>);
    CAV_PASS(eq<pack<t1, t5>, flat_pack_union_t<pack<t1, t1>, pack<t5>>>);
    CAV_PASS(eq<pack<t5, t1>, flat_pack_union_t<pack<t5, t1>, pack<t1>>>);
    CAV_PASS(eq<pack<t1, t5>, flat_pack_union_t<pack<>, pack<t1, t1>, t5>>);
}  // namespace
#endif


////// remove_from_pack ////// @brief
namespace detail {
    template <typename AccT, typename... Ts>
    struct remove_type_impl {
        static_assert(sizeof...(Ts) == 1);
        using type = AccT;
    };

    template <typename... AccTs, typename RemT, typename T, typename... Ts>
    struct remove_type_impl<pack<AccTs...>, RemT, T, Ts...>
        : remove_type_impl<pack<AccTs..., T>, RemT, Ts...> {};

    template <typename... AccTs, typename RemT, typename... Ts>
    struct remove_type_impl<pack<AccTs...>, RemT, RemT, Ts...>
        : remove_type_impl<pack<AccTs...>, RemT, Ts...> {};

}  // namespace detail

template <typename RemT, typename... Ts>
struct remove_type : detail::remove_type_impl<pack<>, RemT, Ts...> {};

template <typename RemT, typename... Ts>
using remove_type_t = typename remove_type<RemT, Ts...>::type;

template <typename PackT, typename RemT>
struct remove_type_unwrap;

template <template <class...> class Tmpl, typename... Ts, typename RemT>
struct remove_type_unwrap<Tmpl<Ts...>, RemT> : remove_type<RemT, Ts...> {};

template <typename PackT, typename RemT>
using remove_type_unwrap_t = typename remove_type_unwrap<PackT, RemT>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<remove_type_t<pack<t0>, t0, t1>, pack<t0, t1>>);
    CAV_PASS(eq<remove_type_t<pack<t1>, t0, t1>, pack<t0, t1>>);
    CAV_PASS(eq<remove_type_unwrap_t<pack<t0, t1, t2, t0, t1>, t0>, pack<t1, t2, t1>>);
    CAV_PASS(eq<remove_type_unwrap_t<pack<t0, t0, t0>, t0>, pack<>>);
}  // namespace
#endif

////// collapse_if_one //////
template <typename T>
struct collapse_if_one;

template <typename T>
struct collapse_if_one<pack<T>> {
    using type = T;
};

template <typename... Ts>
struct collapse_if_one<pack<Ts...>> {
    using type = pack<Ts...>;
};

template <typename T>
using collapse_if_one_t = typename collapse_if_one<T>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<collapse_if_one_t<pack<void>>, void>);
    CAV_PASS(eq<collapse_if_one_t<pack<void, t1>>, pack<void, t1>>);
}  // namespace
#endif

////// pack difference //////
namespace detail {
    template <typename AccT, typename T1, typename>
    struct pack_difference_impl {
        static_assert(eq<T1, pack<>>, "'pack_difference assumes' that it is working with 'pack'");
        using type = AccT;
    };

    template <typename... AccTs, typename T, typename... T1s, typename PackT2>
    struct pack_difference_impl<pack<AccTs...>, pack<T, T1s...>, PackT2>
        : pack_difference_impl<pack<AccTs..., T>, pack<T1s...>, PackT2> {};

    template <typename... AccTs, typename T, typename... T1s, typename PackT2>
    requires(has_type_unwrap_v<T, PackT2>)
    struct pack_difference_impl<pack<AccTs...>, pack<T, T1s...>, PackT2>
        : pack_difference_impl<pack<AccTs...>, pack<T1s...>, PackT2> {};

}  // namespace detail

template <typename PackT1, typename PackT2>
struct pack_difference : detail::pack_difference_impl<pack<>, PackT1, PackT2> {};

template <typename PackT1, typename PackT2>
using pack_difference_t = typename detail::pack_difference_impl<pack<>, PackT1, PackT2>::type;


#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<pack_difference_t<pack<t1, t2, t3, t4>, pack<t1, t4>>, pack<t2, t3>>);
    CAV_PASS(eq<pack_difference_t<pack<t1, t1, t1, t1>, pack<t1, t4>>, pack<>>);
    CAV_PASS(eq<pack_difference_t<pack<t1, t2>, pack<t3, t4>>, pack<t1, t2>>);
    CAV_PASS(eq<pack_difference_t<pack<>, pack<t3, t4>>, pack<>>);
}  // namespace
#endif

////// not_copy_move_ctor //////
/// @brief Exclude copy and move constructor when using forwarding references.
template <typename T, typename... Ts>
concept not_copy_move_ctor = (sizeof...(Ts) > 1 || !eq_no_cvr<first_type_t<Ts...>, T>);

//////// copy type qualifiers between types //////
template <typename FromT, typename ToT>
struct copy_const {
    using type = ToT;
};

template <typename FromT, typename ToT>
struct copy_const<const FromT, ToT> {
    using type = const ToT;
};

template <typename FromT, typename ToT>
using copy_const_t = typename copy_const<FromT, ToT>::type;

template <typename FromT, typename ToT>
struct copy_vol {
    using type = ToT;
};

template <typename FromT, typename ToT>
struct copy_vol<volatile FromT, ToT> {
    using type = volatile ToT;
};

template <typename FromT, typename ToT>
using copy_vol_t = typename copy_vol<FromT, ToT>::type;

template <typename FromT, typename ToT>
struct copy_ref {
    using type = ToT;
};

template <typename FromT, typename ToT>
struct copy_ref<FromT&, ToT> {
    using type = ToT&;
};

template <typename FromT, typename ToT>
struct copy_ref<FromT&&, ToT> {
    using type = ToT&&;
};

template <typename FromT, typename ToT>
using copy_ref_t = typename copy_ref<FromT, ToT>::type;

template <typename FromT, typename ToT>
struct copy_cvref {
    using from_no_ref = std::remove_reference_t<FromT>;
    using type        = copy_ref_t<FromT, copy_const_t<from_no_ref, copy_vol_t<from_no_ref, ToT>>>;
};

template <typename FromT, typename ToT>
using copy_cvref_t = typename copy_cvref<FromT, ToT>::type;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(eq<copy_const_t<char, int>, int>);
    CAV_PASS(eq<copy_const_t<char const, int>, int const>);
    CAV_PASS(eq<copy_vol_t<char, int>, int>);
    CAV_PASS(eq<copy_vol_t<char volatile, int>, int volatile>);
    CAV_PASS(eq<copy_ref_t<char, int>, int>);
    CAV_PASS(eq<copy_ref_t<char&, int>, int&>);
    CAV_PASS(eq<copy_ref_t<char&&, int>, int&&>);
    CAV_PASS(eq<copy_cvref_t<char, int>, int>);
    CAV_PASS(eq<copy_cvref_t<char const, int>, int const>);
    CAV_PASS(eq<copy_cvref_t<char volatile, int>, int volatile>);
    CAV_PASS(eq<copy_cvref_t<char const&, int>, int const&>);
    CAV_PASS(eq<copy_cvref_t<char const&&, int>, int const&&>);
    CAV_PASS(eq<copy_cvref_t<char const volatile&&, int>, int const volatile&&>);
}  // namespace
#endif


}  // namespace cav

#endif /* CAV_INCLUDE_MP_UTILS_HPP */
