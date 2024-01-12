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

#ifdef CAV_INCLUDE_UTILS_TUPLE_UTILS_HPP
#define CAV_INCLUDE_UTILS_TUPLE_UTILS_HPP

#include <fmt/core.h>

#include <utility>

#include "External/tuplet/tuple.hpp"  //
#include "instance_of.hpp"      // IWYU pragma: keep # inst_of
#include "util_functions.hpp"   //

namespace cav {
template <typename... Args>
struct tuple;

template <template <class...> class WrapTup, template <class...> class BaseTup, typename... Args>
constexpr auto tup_base_to_wrapper(BaseTup<Args...>&& t) {
    return WrapTup<Args...>{std::move(t)};
}

template <template <class...> class WrapTup, template <class...> class BaseTup, typename... Args>
constexpr auto tup_base_to_wrapper(BaseTup<Args...> const& t) {
    return WrapTup<Args...>{t};
}

template <typename... TupTs>
requires(inst_of<TupTs, tuple> && ...)
constexpr auto tuple_cat(TupTs&&... args) {
    return tup_base_to_wrapper<tuple>(tuplet::tuple_cat(FWD(args)...));
}

template <typename... Args>
constexpr auto make_tuple(Args&&... args) {
    return tuple<no_cvr<Args>...>{FWD(args)...};
}

// Proxy for tuple, I'll probably try other tuple implementations at some point
// Note: not an alias otherwise operator+= should be declared at global scope
template <typename... Args>
struct tuple : tuplet::tuple<Args...> {
    using base = tuplet::tuple<Args...>;

    // Side-effects only, apply lambda to each element
    using base::for_each;

    // Apply lambda to each element and return another tuple
    auto transform(auto&& lambda) & {
        return reduce([&](auto&... elems) { return ::cav::make_tuple(FWD(lambda)(elems)...); });
    }

    auto transform(auto&& lambda) const& {
        return reduce(
            [&](auto const&... elems) { return ::cav::make_tuple(FWD(lambda)(elems)...); });
    }

    auto transform(auto&& lambda) && {
        return reduce(
            [&](auto&&... elems) { return ::cav::make_tuple(FWD(lambda)(FWD(elems))...); });
    }

    // Apply lambda to each element and return an array. Lambda must return always the same type
    auto transform_to_array(auto&& lambda) & {
        return reduce([&](auto&... elem) { return std::array{FWD(lambda)(elem)...}; });
    }

    auto transform_to_array(auto&& lambda) const& {
        return reduce([&](auto const&... elem) { return std::array{FWD(lambda)(elem)...}; });
    }

    auto transform_to_array(auto&& lambda) && {
        return reduce([&](auto&&... elem) { return std::array{FWD(lambda)(FWD(elem))...}; });
    }

    // As transform, but the lambda receive all the elements of the tuple as a pack
    auto reduce(auto&& lambda) & {
        return tuplet::apply(FWD(lambda), *this);
    }

    auto reduce(auto&& lambda) const& {
        return tuplet::apply(FWD(lambda), *this);
    }

    auto reduce(auto&& lambda) && {
        return tuplet::apply(FWD(lambda), std::move(*this));
    }

    [[nodiscard]] auto& first() {
        return get<0>(*this);
    }

    [[nodiscard]] auto const& first() const {
        return get<0>(*this);
    }

    [[nodiscard]] auto& last() {
        return get<size() - 1>(*this);
    }

    [[nodiscard]] auto const& last() const {
        return get<size() - 1>(*this);
    }

    [[nodiscard]] static constexpr int size() {
        return sizeof...(Args);
    }

    [[nodiscard]] static constexpr bool empty() {
        return size() == 0;
    }
};

template <typename... Args>
tuple(Args&&...) -> tuple<no_cvr<Args>...>;

/// @brief Concatenate tuples into a single tuple
template <typename... Tuples>
struct cat_tuple {
    using type = TYPEOF(tuple_cat(::std::declval<Tuples>()...));
};

template <typename... Tuples>
using cat_tuple_t = typename cat_tuple<Tuples...>::type;

/// @brief Permute tuple elements following indexes parameter pack.
///
/// @tparam TupT
/// @tparam indexes
template <typename TupT, int... Ids>
struct tuple_permute {
    static_assert(!(sizeof...(Ids) < std::tuple_size<TupT>{}),
                  "Too few permutation indexes have been provided.");
    static_assert(!(sizeof...(Ids) > std::tuple_size<TupT>{}),
                  "Too many permutation indexes have been provided.");

    using type = tuple<std::tuple_element_t<Ids, TupT>...>;
};

template <typename TupT, int... Ids>
using tuple_permute_t = typename tuple_permute<TupT, Ids...>::type;


/// @brief Return a tuple containing the types of the
/// parameter pack but the void elements.
///
template <typename... Ts>
struct make_tuple_no_void_helper;

template <typename... Ts1>
struct make_tuple_no_void_helper<tuple<Ts1...>> {
    using type = tuple<Ts1...>;
};

template <typename... Ts1, typename T, typename... Ts2>
struct make_tuple_no_void_helper<tuple<Ts1...>, T, Ts2...> {
    using type = typename make_tuple_no_void_helper<tuple<Ts1..., T>, Ts2...>::type;
};

template <typename... Ts1, typename... Ts2>
struct make_tuple_no_void_helper<tuple<Ts1...>, void, Ts2...> {
    using type = typename make_tuple_no_void_helper<tuple<Ts1...>, Ts2...>::type;
};

template <typename... Ts>
struct make_tuple_no_void {
    using type = typename make_tuple_no_void_helper<tuple<>, Ts...>::type;
};

template <typename... Ts>
using make_tuple_no_void_t = typename make_tuple_no_void<Ts...>::type;

namespace detail {
    template <size_t Id, typename TupT>
    requires(Id >= no_cvr<TupT>::size())  // Define the undef behaviour -> visit last
    [[nodiscard]] constexpr auto visit_idx_helper(TupT&& tup, size_t /*idx*/, auto&& lambda) {
        return FWD(lambda)(::tuplet::get<no_cvr<TupT>::size() - 1>(FWD(tup)));
    }

    template <size_t Id, typename TupT>
    requires(Id < no_cvr<TupT>::size())
    [[nodiscard]] constexpr auto visit_idx_helper(TupT&& tup, size_t idx, auto&& lambda) {
        if (Id < idx)
            return visit_idx_helper<Id + 1>(FWD(tup), idx, FWD(lambda));
        assert(Id == idx);
        return FWD(lambda)(::tuplet::get<Id>(FWD(tup)));
    }
}  // namespace detail

template <typename... Ts>
[[nodiscard]] constexpr auto visit_idx(tuple<Ts...>& tup, size_t idx, auto&& lambda) {
    assert(idx < sizeof...(Ts) && "Index out of tuple bound.");
    return detail::visit_idx_helper<0>(tup, idx, FWD(lambda));
}

template <typename... Ts>
[[nodiscard]] constexpr auto visit_idx(tuple<Ts...> const& tup, size_t idx, auto&& lambda) {
    assert(idx < sizeof...(Ts) && "Index out of tuple bound.");
    return detail::visit_idx_helper<0>(tup, idx, FWD(lambda));
}

/// @brief Element wise op
template <typename... Args>
tuple<Args...> elem_wise_op(tuple<Args...> const& t1, tuple<Args...> const& t2, auto&& lambda) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...> /*is*/) {
        return tuple<Args...>{FWD(lambda)(get<Is>(t1), get<Is>(t2))...};
    }(std::index_sequence_for<Args...>());
}

template <typename... Args>
tuple<Args...>& elem_wise_inplace_op(tuple<Args...>& t1, tuple<Args...> const& t2, auto&& lambda) {
    [&]<std::size_t... Is>(std::index_sequence<Is...> /*is*/) {
        (FWD(lambda)(get<Is>(t1), get<Is>(t2)), ...);
    }(std::index_sequence_for<Args...>());
    return t1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Element wise mathematical operations for tuple pairs ///////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Args>
tuple<Args...> operator+(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, std::plus<>{});
}

template <typename... Args>
tuple<Args...>& operator+=(tuple<Args...>& t1, tuple<Args...> const& t2) {
    return elem_wise_inplace_op(t1, t2, [](auto& v1, auto const& v2) { return v1 += v2; });
}

template <typename... Args>
tuple<Args...> operator-(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, std::minus<>{});
}

template <typename... Args>
tuple<Args...>& operator-=(tuple<Args...>& t1, tuple<Args...> const& t2) {
    return elem_wise_inplace_op(t1, t2, [](auto& v1, auto const& v2) { return v1 -= v2; });
}

template <typename... Args>
tuple<Args...> operator*(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, std::multiplies<>{});
}

template <typename... Args>
tuple<Args...>& operator*=(tuple<Args...>& t1, tuple<Args...> const& t2) {
    return elem_wise_inplace_op(t1, t2, [](auto& v1, auto const& v2) { return v1 *= v2; });
}

template <typename... Args>
tuple<Args...> operator/(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, std::divides<>{});
}

template <typename... Args>
tuple<Args...>& operator/=(tuple<Args...>& t1, tuple<Args...> const& t2) {
    return elem_wise_inplace_op(t1, t2, [](auto& v1, auto const& v2) { return v1 /= v2; });
}

// Specialization for tuples
template <typename... Args>
tuple<Args...> max(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, [](auto const& v1, auto const& v2) { return std::max(v1, v2); });
}

template <typename... Args>
tuple<Args...> min(tuple<Args...> const& t1, tuple<Args...> const& t2) {
    return elem_wise_op(t1, t2, [](auto const& v1, auto const& v2) { return std::min(v1, v2); });
}

template <typename... Args>
tuple<Args...> abs(tuple<Args...> const& t1) {
    return t1.transform([&]<class T>(T const& elem) -> T { return abs(elem); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////// Element wise mathematical operations between tuples and scalars //////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Args>
tuple<Args...> operator+(tuple<Args...> const& t1, arithmetic auto const& val) {
    return t1.transform([&]<class T>(T const& elem) -> T { return elem + val; });
}

template <typename... Args>
tuple<Args...>& operator+=(tuple<Args...>& t1, arithmetic auto const& val) {
    t1.for_each([&](auto& elem) { return elem += val; });
    return t1;
}

/// @brief Element wise subtraction for tuples
template <typename... Args>
tuple<Args...> operator-(tuple<Args...> const& t1, arithmetic auto const& val) {
    return t1.transform([&]<class T>(T const& elem) -> T { return elem - val; });
}

template <typename... Args>
tuple<Args...>& operator-=(tuple<Args...>& t1, arithmetic auto const& val) {
    t1.for_each([&](auto& elem) { return elem -= val; });
    return t1;
}

/// @brief Element wise product for tuples
template <typename... Args>
tuple<Args...> operator*(tuple<Args...> const& t1, arithmetic auto const& val) {
    return t1.transform([&]<class T>(T const& elem) -> T { return elem * val; });
}

template <typename... Args>
tuple<Args...>& operator*=(tuple<Args...>& t1, arithmetic auto const& val) {
    t1.for_each([&](auto& elem) { return elem *= val; });
    return t1;
}

/// @brief Element wise division for tuples
template <typename... Args>
tuple<Args...> operator/(tuple<Args...> const& t1, arithmetic auto const& val) {
    return t1.transform([&]<class T>(T const& elem) -> T { return elem * val; });
}

template <typename... Args>
tuple<Args...>& operator/=(tuple<Args...>& t1, arithmetic auto const& val) {
    t1.for_each([&](auto& elem) { return elem /= val; });
    return t1;
}

/// @brief Element wise power for tuples
template <typename... Args>
tuple<Args...> pow(tuple<Args...> const& t, double exp) {
    return t.transform([&]<class T>(T const& elem) -> T { return pow(elem, exp); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// UTILITIES /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

template <not_inst_of<tuple> T>
constexpr auto tuple_merge_into(T&& t1, auto&&... ts);

template <typename TupT>
requires inst_of<TupT, tuple>
constexpr TupT&& tuple_merge_into(TupT&& tup) {
    return FWD(tup);
}

template <not_inst_of<tuple> T>
constexpr auto tuple_merge_into(T&& t) {
    return cav::make_tuple(FWD(t));
}

template <typename TupT>
requires inst_of<TupT, tuple>
constexpr auto tuple_merge_into(TupT&& tup1, auto&&... ts) {
    return cav::tuple_cat(FWD(tup1), tuple_merge_into(FWD(ts)...));
}

template <not_inst_of<tuple> T>
constexpr auto tuple_merge_into(T&& t1, auto&&... ts) {
    return cav::tuple_cat(cav::make_tuple(FWD(t1)), tuple_merge_into(FWD(ts)...));
}

template <typename... Ts>
struct merge_into_tuple {
    using type = TYPEOF(tuple_merge_into(std::declval<Ts>()...));
};

template <typename... Ts>
using merge_into_tuple_t = typename merge_into_tuple<Ts...>::type;

static_assert(std::is_same_v<tuple<int, double>, TYPEOF(tuple_merge_into(1, 3.0))>);
static_assert(std::is_same_v<tuple<int, float, double>,
                             TYPEOF(tuple_merge_into(1, cav::make_tuple(3.1F, 3.4)))>);
static_assert(std::is_same_v<tuple<int64_t, double, long double>,
                             TYPEOF(tuple_merge_into(cav::make_tuple(3L, 3.3), 3.4L))>);
static_assert(
    std::is_same_v<tuple<int64_t, double, float, double>,
                   TYPEOF(tuple_merge_into(cav::make_tuple(3L, 3.3), cav::make_tuple(3.1F, 3.4)))>);

template <typename Tup, template <class...> class FtorT>
struct for_each_type;

template <typename... TupTs, template <class...> class FtorT>
struct for_each_type<tuple<TupTs...>, FtorT> {
    using type                  = typename FtorT<TupTs...>::type;
    static constexpr auto value = FtorT<TupTs...>::value;
};

}  // namespace cav

namespace std {
template <class... T>
struct tuple_size<cav::tuple<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

template <size_t I, class... T>
struct tuple_element<I, cav::tuple<T...>> {
    using type = TYPEOF(tuplet::get<I>(std::declval<cav::tuple<T...>>()));
};

template <size_t I, typename TupT>
using tuple_element_t = typename tuple_element<I, TupT>::type;
}  // namespace std

#endif /* CAV_INCLUDE_UTILS_TUPLE_UTILS_HPP */
