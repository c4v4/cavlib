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

#ifndef CAV_INCLUDE_UTIL_FUNCTIONS_HPP
#define CAV_INCLUDE_UTIL_FUNCTIONS_HPP

#include <cassert>
#include <compare>
#include <concepts>
#include <cstdint>
#include <span>

#include "../comptime/macros.hpp"
#include "../comptime/mp_utils.hpp"
#include "../comptime/syntactic_sugars.hpp"
#include "../numeric/limits.hpp"
#include "cav/vectors/OwnSpan.hpp"

namespace cav {

////////////////////////////////// SOME UTILITY OR MATH FUNCTIONS //////////////////////////////////

template <std::integral T1, std::integral T2>
[[nodiscard]] CAV_CONST constexpr bool check_overflow_sum(T1 i1, T2 i2) {
    using int_type = std::common_type_t<T1, T2>;
    return (i2 > 0 && i1 > type_max<int_type> - i2) || (i2 < 0 && i1 < type_min<int_type> - i2);
}

template <std::integral T1, std::integral T2>
[[nodiscard]] CAV_CONST constexpr bool check_overflow_dif(T1 i1, T2 i2) {
    using int_type = std::common_type_t<T1, T2>;
    return (i2 < 0 && i1 > type_max<int_type> + i2) || (i2 > 0 && i1 < type_min<int_type> + i2);
}

template <std::integral T1, std::integral T2>
[[nodiscard]] CAV_CONST constexpr bool check_overflow_mul(T1 i1, T2 i2) {
    using int_type = std::common_type_t<T1, T2>;
    if (i1 == -1)
        return i2 == type_min<int_type>;
    if (i2 == -1)
        return i1 == type_min<int_type>;
    if (i1 != 0)
        return i2 > type_max<int_type> / i1 || i2 < type_min<int_type> / i1;
    return false;
}

template <std::integral T1, std::integral T2>
[[nodiscard]] CAV_CONST constexpr bool check_overflow_div(T1 i1, T2 i2) {
    using int_type = std::common_type_t<T1, T2>;
    return i1 == type_min<int_type> && i2 == -1;
}

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(check_overflow_sum(1 << 30, 1 << 30));
    CAV_PASS(!check_overflow_sum(1 << 30, 1 << 29));
    CAV_PASS(check_overflow_sum(type_max<int>, 1));
    CAV_PASS(check_overflow_sum(1, type_max<int>));
    CAV_PASS(!check_overflow_sum(type_min<int>, type_max<int>));

    CAV_PASS(check_overflow_dif(type_max<int>, -type_max<int>));
    CAV_PASS(!check_overflow_dif(0, -type_max<int> + 1));
    CAV_PASS(check_overflow_dif(type_max<int>, -1));
    CAV_PASS(check_overflow_dif(-2, type_max<int>));
    CAV_PASS(!check_overflow_dif(type_min<int>, -type_max<int>));

    CAV_PASS(check_overflow_mul(1 << 16, 1 << 16));
    CAV_PASS(!check_overflow_mul(1 << 16, (1 << 15) - 1));
    CAV_PASS(check_overflow_mul(-(type_min<int> / 2), 2));
    CAV_PASS(check_overflow_mul(2, -(type_min<int> / 2)));
    CAV_PASS(!check_overflow_mul(-1, 1));
}  // namespace
#endif

[[nodiscard]] CAV_PURE constexpr int ssize(auto&& range) noexcept {
    return static_cast<int>(range.size());
}

[[nodiscard]] CAV_CONST constexpr bool max(bool b1, bool b2) noexcept {
    return b1 || b2;
}

template <typename T>
[[nodiscard]] CAV_PURE constexpr auto max(T const& v1, T const& v2) noexcept {
    return v1 >= v2 ? v1 : v2;
}

[[nodiscard]] CAV_PURE constexpr auto max(auto const& v1, auto const& v2) noexcept {
    return v1 >= v2 ? v1 : static_cast<TYPEOF(v1)>(v2);
}

[[nodiscard]] CAV_CONST constexpr bool min(bool b1, bool b2) noexcept {
    return b1 && b2;
}

template <typename T>
[[nodiscard]] CAV_PURE constexpr auto min(T const& v1, T const& v2) noexcept {
    return v1 <= v2 ? v1 : v2;
}

[[nodiscard]] CAV_PURE constexpr auto min(auto const& v1, auto const& v2) noexcept {
    return v1 <= v2 ? v1 : static_cast<TYPEOF(v1)>(v2);
}

[[nodiscard]] CAV_PURE constexpr auto clip(auto const& v, auto const& lb, auto const& ub) noexcept {
    return (lb <= v && v <= ub) ? v : (v < lb) ? lb : ub;
}

/// @brief Compute the square of a given number;
[[nodiscard]] CAV_CONST constexpr double sqr(double x) noexcept {
    return x * x;
}

template <typename T>
[[nodiscard]] CAV_CONST constexpr T abs(T val) noexcept {
    return val < T{} ? -val : val;
}

template <typename T>
[[nodiscard]] CAV_CONST constexpr bool is_positive(T val) noexcept {
    return val >= T{};
}

template <typename T>
[[nodiscard]] CAV_CONST constexpr bool is_reversed(T dim) noexcept {
    return dim < T{};
}

[[nodiscard]] CAV_CONST constexpr int rev_to_sign(auto rev) noexcept {
    return rev ? -1 : 1;
}

[[nodiscard]] CAV_CONST constexpr int dim_to_sign(auto dim) noexcept {
    return dim < 0 ? -1 : 1;
}

[[nodiscard]] CAV_CONST constexpr bool is_even(std::integral auto n) noexcept {
    return (n & 1) == 0;
}

[[nodiscard]] CAV_CONST constexpr bool is_odd(std::integral auto n) noexcept {
    return (n & 1) != 0;
}

[[nodiscard]] CAV_CONST static constexpr auto lshift(std::integral auto val,
                                                     std::integral auto amount) {
    if (amount < 0)
        return val >> amount;
    return val << amount;
}

[[nodiscard]] CAV_CONST static constexpr auto rshift(std::integral auto val,
                                                     std::integral auto amount) {
    if (amount < 0)
        return val << amount;
    return val >> amount;
}

template <std::integral IntT = int64_t>
[[nodiscard]] CAV_CONST constexpr IntT pow(std::integral auto base_arg,
                                           std::integral auto exp_arg) noexcept {
    assert(exp_arg >= 0);
    assert(exp_arg <= 64);
    assert(base_arg >= 0);
    uint64_t base   = base_arg;
    int8_t   exp    = exp_arg;
    IntT     result = 1;
    for (; exp != 0; exp /= 2) {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
    }
    return result;
}

template <std::floating_point T>
[[nodiscard]] CAV_CONST constexpr T pow(T base, int64_t exp_arg) noexcept {
    T result = static_cast<T>(1);
    for (uint8_t exp = abs(exp_arg); exp != 0ULL; exp--)
        result *= base;
    if (exp_arg < 0)
        return static_cast<T>(1) / result;
    return result;
}

template <std::integral IntT>
[[nodiscard]] CAV_CONST constexpr bool pow_oveflow_check(std::integral auto base_arg,
                                                         std::integral auto exp_arg) noexcept {
    IntT base = static_cast<IntT>(base_arg);
    IntT exp  = static_cast<IntT>(exp_arg);

    IntT result = 1ULL;
    if (exp == 0ULL || base == 0ULL)
        return true;
    for (;;) {
        IntT max = type_max<IntT> / base;

        if (result > max)
            return false;
        if (exp % 2ULL == 1ULL)
            result *= base;

        exp >>= 1ULL;
        if (exp == 0ULL)
            return true;

        if (base > max)
            return false;
        base *= base;
    }
    return true;
}

namespace detail {
    template <size_t I, template <class...> class TupT, typename... Ts>
    constexpr std::partial_ordering partial_compare_impl(TupT<Ts...> const&    t1,
                                                         TupT<Ts...> const&    t2,
                                                         std::partial_ordering part_result) {
        if constexpr (I == sizeof...(Ts))
            return part_result;
        else {
            auto t1t2_cmp = get<I>(t1) <=> get<I>(t2);
            if (std::is_eq(part_result))
                return partial_compare_impl<I + 1>(t1, t2, t1t2_cmp);
            if (std::is_eq(t1t2_cmp) || t1t2_cmp == part_result)
                return partial_compare_impl<I + 1>(t1, t2, part_result);

            assert(t1t2_cmp != part_result);
            return std::partial_ordering::unordered;
        }
    }
}  // namespace detail

/// @brief Partial comparison of tuple-like objects.
/// Given two objects t1 and t2 of the same type:
/// - t1 > t2 (resp. <) if exists a corresponding subobject s1 of t1 and s2 of
/// t2 where s1 > s2 (resp. <), while all the other subobjects of t1 are >=
/// (resp. <=) to their corresponding subobjects of t2.
/// - t1 == t2 if every subobject of t1 is == to the corresponding subobjects of
/// t2.
template <template <class...> class TupT, typename... Ts>
[[nodiscard]] constexpr std::partial_ordering partial_compare(TupT<Ts...> const& t1,
                                                              TupT<Ts...> const& t2) {
    return detail::partial_compare_impl<1>(t1, t2, get<0>(t1) <=> get<0>(t2));
}

template <typename SbT = ct<0>, typename SeT = ct<type_max<int>>>
[[nodiscard]] CAV_PURE constexpr auto subspan(auto&& container,
                                              int    skip_beg = {},
                                              int    skip_end = {}) {
    int c_size = std::ssize(container);
    int b_idx  = clip(skip_beg + (skip_beg >= 0 ? 0 : c_size), 0, c_size);
    int e_idx  = clip(skip_end + (skip_end >= 0 ? 0 : c_size), 0, c_size);

    if (b_idx > e_idx)
        return std::span{std::begin(container), std::begin(container)};
    return std::span{std::begin(container) + b_idx, std::begin(container) + e_idx};
}

template <int SkipB, int SkipE = type_max<int>>  // legacy
[[nodiscard]] CAV_PURE constexpr auto subspan(auto&& container) {
    return subspan(FWD(container), ct_v<SkipB>, ct_v<SkipE>);
}

template <typename ItT>
struct iterator_pair {
    ItT begin;
    ItT end;
};

template <typename ItT>
iterator_pair(ItT, ItT) -> iterator_pair<ItT>;

template <typename ContainerT>
[[nodiscard]] CAV_PURE constexpr auto get_begin_end(ContainerT&& cont) noexcept {
    return iterator_pair{cont.begin(), cont.end()};
}

/// @brief Integer constexpt base 10 log
template <std::integral T>
[[nodiscard]] constexpr auto ilog10(T val) {
    T result = 0;
    while ((val /= 10) != 0)
        ++result;
    return result;
};

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(ilog10(9) == 0);
    CAV_PASS(ilog10(10) == 1);
    CAV_PASS(ilog10(99) == 1);
    CAV_PASS(ilog10(100) == 2);
    CAV_PASS(ilog10(999) == 2);
    CAV_PASS(ilog10(1000) == 3);
}  // namespace
#endif

/// @brief Integer constexpr sqrt
template <std::integral T>
[[nodiscard]] constexpr T isqrt(T x) {
    T low  = 0;
    T high = x / 2 + 1;

    while (high != low) {
        auto mid_point = (low + high + 1) / 2;
        if (x / mid_point < mid_point)
            high = mid_point - 1;
        else
            low = mid_point;
    }
    return low;
}

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(isqrt(1ULL << 62ULL) == (1ULL << 31ULL));
    CAV_PASS(isqrt(~0ULL) == (~0U));
    CAV_PASS(isqrt(620607744) == (24912));
}  // namespace
#endif

/// Simple range stuff

template <typename T>
constexpr void iota(auto& container, T start, T const& step = 1) noexcept {
    for (size_t i = 0; i < container.size(); ++i) {
        container[i] = start;
        start += step;
    }
}

template <typename T>
[[nodiscard]] constexpr cav::OwnSpan<T> make_iota(T start, T end, T const& step = 1) noexcept {
    auto container = cav::OwnSpan<T>((end - start) / step);
    cav::iota(container, start, step);
    return container;
}

template <template <class...> class ContTmpl, typename T>
[[nodiscard]] constexpr ContTmpl<T> make_iota(T start, T end, T const& step = 1) noexcept {
    auto container = ContTmpl<T>((end - start) / step);
    cav::iota(container, start, step);
    return container;
}

template <typename ContT, typename T>
[[nodiscard]] constexpr ContT make_iota(T start, T end, T step = 1) noexcept {
    auto container = ContT((end - start) / step);
    cav::iota(container, start, step);
    return container;
}

[[nodiscard]] constexpr size_t find_idx(auto const& container, auto const& value) {
    for (size_t i = 0; i < container.size(); ++i)
        if (container[i] == value)
            return i;
    return container.size();
}

[[nodiscard]] constexpr auto max_elem(auto const& container) {
    if (std::empty(container))
        return cav::no_cvr<decltype(*container.begin())>{};

    auto max_e = *container.begin();
    for (auto const& elem : subspan<1>(container))
        max_e = cav::max(max_e, elem);
    return max_e;
}

[[nodiscard]] constexpr auto min_elem(auto const& container) {
    if (std::empty(container))
        return cav::no_cvr<decltype(*container.begin())>{};

    auto max_e = *container.begin();
    for (auto const& elem : subspan<1>(container))
        max_e = cav::min(max_e, elem);
    return max_e;
}

[[nodiscard]] constexpr decl_auto first_elem(auto&& a1, auto&&... /*args*/) {
    return FWD(a1);
}

namespace detail {
    constexpr decl_auto identity(auto&& x) {
        return FWD(x);
    }
}  // namespace detail

template <auto Default>
[[nodiscard]] constexpr decl_auto last_elem(auto&&... args) {
    if constexpr (sizeof...(args) == 0)
        return Default;
    else
        return (detail::identity(FWD(args)), ...);
}

[[nodiscard]] constexpr decl_auto last_elem(auto&&... args) {
    return (detail::identity(FWD(args)), ...);
}

/// @brief In some places C++ does not applies the implicit conversion that otherwise applies in any
/// other place. Less implicit conversion is good, but not when it break what one expects, like when
/// one use aggregates and double cannot be casted into float for example when using aggregate
/// initialization. This cast simply use the implicit cast that works in the other places (making in
/// essence an explicit-cast that works only when the implicit cast works).
/// @tparam To
/// @tparam From
template <typename To, typename From>
requires std::is_compound_v<From>
copy_ref_t<From, To> implicit_cast(From&& x) {
    return FWD(x);
}

template <typename To, typename From>
requires std::is_fundamental_v<From>
To implicit_cast(From x) {
    return x;
}

#ifdef CAV_COMP_TESTS
namespace {
    // iota
    CAV_BLOCK_PASS({
        auto ospan = cav::OwnSpan{5, 0};
        cav::iota(ospan, 10, 2);
        for (int i = 10; auto x : ospan) {
            assert(x == i);
            i += 2;
        }
    });

    CAV_BLOCK_FAIL({
        auto ospan = cav::OwnSpan{5, 0};
        cav::iota(ospan, 10, 2);
        for (int i = 10; auto x : ospan) {
            i += 2;  // wrong increment position
            assert(x == i);
        }
    });

    // make_iota
    CAV_BLOCK_PASS({
        auto ospan = cav::make_iota(10, 20, 2);
        for (int i = 10; auto x : ospan) {
            assert(x == i);
            i += 2;
        }
    });

    CAV_BLOCK_PASS({
        auto ospan1 = cav::OwnSpan{5, 0};
        cav::iota(ospan1, 10, 2);
        auto ospan2 = cav::make_iota<cav::OwnSpan>(10, 20, 2);
        for (int i = 0; i < 5; ++i)
            assert(ospan1[i] == ospan2[i]);
    });

    CAV_BLOCK_FAIL({
        auto ospan = cav::make_iota<cav::OwnSpan<int>>(10, 20, 2);
        for (int i = 10; auto x : ospan) {
            i += 2;  // wrong increment position
            assert(x == i);
        }
    });

    // find_idx
    CAV_PASS(find_idx(std::array{0, 1, 18, 3}, 18) == 2);
    CAV_PASS(find_idx(std::array{0, 1, 18, 3}, 17) == 4);

    // max_elem min_elem
    CAV_PASS(max_elem(std::array{0, 1, 18, 3}) == 18);
    CAV_PASS(min_elem(std::array{0, 1, 18, 3}) == 0);
    CAV_FAIL(max_elem(std::array{0, 1, -18, 3}) == 0);
    CAV_FAIL(min_elem(std::array{100, -1, 18, 3}) == 0);


    // first_elem
    constexpr inline int   a = 1, b = 2, c = 3;
    constexpr inline float d = 1.0, e = 2.0, f = 3.0;

    CAV_PASS(first_elem(1, 2, 3) == 1);
    CAV_PASS(first_elem(1.0, 2U, 3.3F, 5LLU) == 1.0);
    CAV_PASS(cav::eq<decltype(first_elem(1, 2, 3)), int&&>);
    CAV_PASS(cav::eq<decltype(first_elem(1.0, 2U, 3.3F, 5LLU)), double&&>);

    CAV_PASS(first_elem(a, b, c) == 1);
    CAV_PASS(first_elem(a, b, c, d, e, f) == 1.0);
    CAV_PASS(cav::eq<decltype(first_elem(a, b, c)), int const&>);
    CAV_PASS(cav::eq<decltype(first_elem(d, e, f, a, b, c)), float const&>);


    // last_elem
    CAV_PASS(last_elem(1, 2, 3) == 3);
    CAV_PASS(last_elem(1.0, 2U, 3.3F, 5LLU) == 5LLU);
    CAV_PASS(cav::eq<decltype(last_elem(1, 2, 3)), int&&>);
    CAV_PASS(cav::eq<decltype(last_elem(1.0, 2U, 3.3F, 5LLU)), long long unsigned&&>);

    CAV_PASS(last_elem(a, b, c) == 3);
    CAV_PASS(last_elem(a, b, c, d, e, f) == 3.0);
    CAV_PASS(cav::eq<decltype(last_elem(a, b, c)), int const&>);
    CAV_PASS(cav::eq<decltype(last_elem(a, b, c, d, e, f)), float const&>);
}  // namespace
#endif

template <int64_t N>
constexpr void for_each_idx(auto&& fn) {
    if constexpr (N > 0)
        [&fn]<std::size_t... Is>(std::index_sequence<Is...>) {
            (FWD(fn)(ct_v<Is>), ...);
        }(std::make_index_sequence<N>{});
}

constexpr void for_each_elem(auto&& fn, auto&&... elems) {
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (FWD(fn)(ct_v<Is>, FWD(elems)), ...);
    }(std::make_index_sequence<sizeof...(elems)>{});
}

}  // namespace cav

#endif /* CAV_INCLUDE_UTIL_FUNCTIONS_HPP */
