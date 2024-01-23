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

#ifndef CAV_INCLUDE_UTILS_TAGGEDSCALAR_HPP
#define CAV_INCLUDE_UTILS_TAGGEDSCALAR_HPP

#include "../comptime/mp_utils.hpp"
#include "../comptime/syntactic_sugars.hpp"
#include "../mish/util_functions.hpp"
#include "../numeric/limits.hpp"

namespace cav {
/// @brief TaggedScalar wrap a native arithmetic type and define the conversions between
/// TaggedScalar as explicit. The conversion with native types are kept implicit for compatibility.
///
/// Upon an arithmentic operation, the result is checked to avoid overflow, infinite, Nan or any
/// non-wanted edge case that is not relevant for my usage of this type.
template <typename T, typename = UNIQUE_TYPE>
struct TaggedScalar {
    using type = T;
    T value;

    constexpr TaggedScalar(T val)
        : value(val) {
        assert(std::isfinite(val));
    };

    constexpr TaggedScalar()
        : value(){};

    constexpr operator T() const {
        assert(std::isfinite(value));
        return value;
    }

    template <typename T2, typename TagT2>
    explicit constexpr operator TaggedScalar<T2, TagT2>() const {
        assert(std::isfinite(value));
        return value;
    }
};

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator+(TaggedScalar<T, TagT> n) {
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator-(TaggedScalar<T, TagT> n) {
    assert(!check_overflow_mul(n.value, -1));
    assert(std::isfinite(-n.value));
    return {-n.value};
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator++(TaggedScalar<T, TagT>& n) {
    assert(!check_overflow_sum(n.value, 1));
    ++n.value;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator--(TaggedScalar<T, TagT>& n) {
    assert(!check_overflow_dif(n.value, 1));
    --n.value;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator++(TaggedScalar<T, TagT>& n, int) {
    assert(!check_overflow_sum(n.value, 1));
    auto old = n;
    ++n.value;
    assert(std::isfinite(n.value));
    return old;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator--(TaggedScalar<T, TagT>& n, int) {
    assert(!check_overflow_dif(n.value, 1));
    auto old = n;
    --n.value;
    assert(std::isfinite(n.value));
    return old;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator+(TaggedScalar<T, TagT> n, auto other) {
    assert(!check_overflow_sum(n.value, other));
    return {n.value + other};
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator-(TaggedScalar<T, TagT> n, auto other) {
    assert(!check_overflow_dif(n.value, other));
    return {n.value - other};
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator*(TaggedScalar<T, TagT> n, auto other) {
    assert(!check_overflow_mul(n.value, other));
    return {n.value * other};
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator/(TaggedScalar<T, TagT> n, auto other) {
    assert(!check_overflow_div(n.value, other));
    return {n.value / other};
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator+=(TaggedScalar<T, TagT>& n, auto other) {
    assert(!check_overflow_sum(n.value, other));
    n.value += other;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator-=(TaggedScalar<T, TagT>& n, auto other) {
    assert(!check_overflow_dif(n.value, other));
    n.value -= other;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator*=(TaggedScalar<T, TagT>& n, auto other) {
    assert(!check_overflow_mul(n.value, other));
    n.value *= other;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator/=(TaggedScalar<T, TagT>& n, auto other) {
    assert(!check_overflow_div(n.value, other));
    n.value /= other;
    assert(std::isfinite(n.value));
    return n;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator+(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n + other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator-(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n - other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator*(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n * other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT> operator/(TaggedScalar<T, TagT> n, TaggedScalar<T, TagT> other) {
    return n / other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator+=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n += other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator-=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n -= other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator*=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n *= other.value;
}

template <typename T, typename TagT>
constexpr TaggedScalar<T, TagT>& operator/=(TaggedScalar<T, TagT>& n, TaggedScalar<T, TagT> other) {
    return n /= other.value;
}

////////////// EXETEND LIMITS TO SUPPORT SCALEDINT //////////////
template <typename T, typename TagT>
struct limits<TaggedScalar<T, TagT>> : limits<T> {};

}  // namespace cav

#if __has_include(<fmt/core.h>)
#include <fmt/core.h>

namespace fmt {
template <typename T, typename TagT>
struct formatter<cav::TaggedScalar<T, TagT>> : fmt::formatter<T> {
    using base        = fmt::formatter<T>;
    using tagget_type = cav::TaggedScalar<T, TagT>;

    auto format(tagget_type c, format_context& ctx) noexcept {
        return fmt::formatter<T>::format(static_cast<T>(c), ctx);
    }
};
}  // namespace fmt
#endif

#endif /* CAV_INCLUDE_UTILS_TAGGEDSCALAR_HPP */
