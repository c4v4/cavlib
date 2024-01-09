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

#ifndef CAV_INCLUDE_UTILS_TOLFLOAT_HPP
#define CAV_INCLUDE_UTILS_TOLFLOAT_HPP

#include "../comptime/syntactic_sugars.hpp"
#include "../mish/util_functions.hpp"

namespace cav {

// TODO(cava): fix the interaporebaility with native types (arithmetic operations do not work)

template <int Exp, int64_t Base = 10, std::floating_point BaseT = double>
struct TolFloat {
    static constexpr BaseT tol = 1.0 / pow(Base, Exp);
    BaseT                  value;

    constexpr TolFloat() = default;
    constexpr TolFloat(auto val)  // implicit if conversion to BaseT is implicit
        : value(val){};

    template <std::integral auto E, std::integral auto B, std::floating_point BT>
    [[nodiscard]] explicit constexpr operator TolFloat<E, B, BT>() {
        return {value};
    }

    template <typename T>
    [[nodiscard]] explicit constexpr operator T() {
        return static_cast<T>(value);
    }
};

// +
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT>& operator+(TolFloat<E, B, BT>& t) noexcept {
    return t;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT> operator+(TolFloat<E, B, BT> t1,
                                                     TolFloat<E, B, BT> t2) noexcept {
    return {t1.value + t2.value};
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
constexpr TolFloat<E, B, BT>& operator+=(TolFloat<E, B, BT>& t1, TolFloat<E, B, BT> t2) noexcept {
    t1.value += t2.value;
    return t1;
}

// -
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT> operator-(TolFloat<E, B, BT> t) noexcept {
    return {-t.value};
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT> operator-(TolFloat<E, B, BT> t1,
                                                     TolFloat<E, B, BT> t2) noexcept {
    return {t1.value - t2.value};
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT>& operator-=(TolFloat<E, B, BT>& t1,
                                                       TolFloat<E, B, BT>  t2) noexcept {
    t1.value -= t2.value;
    return t1;
}

// *
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT> operator*(TolFloat<E, B, BT> t1,
                                                     TolFloat<E, B, BT> t2) noexcept {
    return {t1.value * t2.value};
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
constexpr TolFloat<E, B, BT>& operator*=(TolFloat<E, B, BT>& t1, TolFloat<E, B, BT> t2) noexcept {
    t1.value *= t2.value;
    return t1;
}

// /
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr TolFloat<E, B, BT> operator/(TolFloat<E, B, BT> t1,
                                                     TolFloat<E, B, BT> t2) noexcept {
    return {t1.value / t2.value};
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
constexpr TolFloat<E, B, BT>& operator/=(TolFloat<E, B, BT>& t1, TolFloat<E, B, BT> t2) noexcept {
    t1.value /= t2.value;
    return t1;
}

// == !=
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator==(auto n, TolFloat<E, B, BT> t2) noexcept {
    return std::abs(n - t2.value) < t2.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator==(TolFloat<E, B, BT> t1, auto n) noexcept {
    return std::abs(t1.value - n) < t1.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator==(TolFloat<E, B, BT> t1, TolFloat<E, B, BT> t2) noexcept {
    return std::abs(t1.value - t2.value) < t1.tol;
}

// >
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>(auto n, TolFloat<E, B, BT> t2) noexcept {
    return n >= t2.value + t2.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>(TolFloat<E, B, BT> t1, auto n) noexcept {
    return t1.value >= n + t1.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>(TolFloat<E, B, BT> t1, TolFloat<E, B, BT> t2) noexcept {
    return t1.value >= t2.value + t1.tol;
}

// >=
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>=(auto n, TolFloat<E, B, BT> t2) noexcept {
    return n > t2.value - t2.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>=(TolFloat<E, B, BT> t1, auto n) noexcept {
    return t1.value > n - t1.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator>=(TolFloat<E, B, BT> t1, TolFloat<E, B, BT> t2) noexcept {
    return t1.value > t2.value - t2.tol;
}

// <
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<(auto n, TolFloat<E, B, BT> t2) noexcept {
    return n <= t2.value - t2.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<(TolFloat<E, B, BT> t1, auto n) noexcept {
    return t1.value <= n - t1.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<(TolFloat<E, B, BT> t1, TolFloat<E, B, BT> t2) noexcept {
    return t1.value <= t2.value - t1.tol;
}

// <=
template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<=(auto n, TolFloat<E, B, BT> t2) noexcept {
    return n < t2.value + t2.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<=(TolFloat<E, B, BT> t1, auto n) noexcept {
    return t1.value < n + t1.tol;
}

template <std::integral auto E, std::integral auto B, std::floating_point BT>
[[nodiscard]] constexpr bool operator<=(TolFloat<E, B, BT> t1, TolFloat<E, B, BT> t2) noexcept {
    return t1.value < t2.value + t2.tol;
}

// NOTE: at the moment comparisons and operations between TolFloat with different precision are not
// supported. Adding support for it is straigth forward, however for the current use of this
// abstraction, these operations would be an error and not having them defined can help avoid this
// class of errors completely.
static_assert(TolFloat<0, 10, float>(1) == TolFloat<0, 10, float>(1));
static_assert(TolFloat<0, 10, float>(1) != TolFloat<0, 10, float>(0));
static_assert(TolFloat<0, 10, float>(1) > TolFloat<0, 10, float>(0));
static_assert(TolFloat<0, 10, float>(1) >= TolFloat<0, 10, float>(0));
static_assert(TolFloat<0, 10, float>(1) >= TolFloat<0, 10, float>(1));
static_assert(TolFloat<0, 10, float>(0) < TolFloat<0, 10, float>(1));
static_assert(TolFloat<0, 10, float>(0) <= TolFloat<0, 10, float>(1));
static_assert(TolFloat<0, 10, float>(0) <= TolFloat<0, 10, float>(0));
static_assert(TolFloat<3, 10, float>(3.134) == TolFloat<3, 10, float>(3.1341));
static_assert(TolFloat<3, 10, float>(3.123) < TolFloat<3, 10, float>(3.124));
static_assert(TolFloat<2, 10, float>(3.134) >= TolFloat<2, 10, float>(3.136));
static_assert(TolFloat<3, 10, float>(3.134) != TolFloat<3, 10, float>(3.136));

}  // namespace cav

#if __has_include(<fmt/format.h>)
#include <fmt/core.h>

namespace fmt {
template <int8_t E, int64_t B, std::floating_point BT>
struct formatter<cav::TolFloat<E, B, BT>> : fmt::formatter<double> {
    auto format(cav::TolFloat<E, B, BT> c, format_context& ctx) noexcept {
        return formatter<double>::format(static_cast<double>(c), ctx);
    }
};
#endif

}  // namespace fmt

#endif /* CAV_INCLUDE_UTILS_TOLFLOAT_HPP */
