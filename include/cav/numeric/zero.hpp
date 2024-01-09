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

#ifndef CAV_INCLUDE_UTILS_ZERO_HPP
#define CAV_INCLUDE_UTILS_ZERO_HPP

#include <concepts>

#include "../comptime/syntactic_sugars.hpp"

namespace cav {

/// @brief Explicit ways to retrieve the default initialized object (0) of a given type.
template <typename T>
[[nodiscard]] constexpr T zero_of(T const& /*v*/) {
    return {};
}

/// @brief Represents the zero (default) value of any default-constructible type. Can be implicitly
/// casted to anything default-constructible.
struct zero_type {
    template <typename T>
    [[nodiscard]] constexpr operator T() const {
        return {};
    }

    [[nodiscard]] constexpr auto operator<=>(zero_type const& /*z*/) const = default;
    [[nodiscard]] constexpr bool operator==(zero_type const& /*z*/) const  = default;

    [[nodiscard]] constexpr zero_type operator+(zero_type const& /*z*/) const {
        return {};
    }

    [[nodiscard]] constexpr zero_type operator-(zero_type const& /*z*/) const {
        return {};
    }

    [[nodiscard]] constexpr zero_type operator*(zero_type const& /*z*/) const {
        return {};
    }

    [[nodiscard]] constexpr zero_type operator/(zero_type const& /*z*/) const {
        return {};
    }

    constexpr zero_type& operator+=(zero_type const& /*z*/) {
        return *this;
    }

    [[nodiscard]] constexpr zero_type& operator-=(zero_type const& /*z*/) {
        return *this;
    }

    constexpr zero_type& operator*=(zero_type const& /*z*/) {
        return *this;
    }

    constexpr zero_type& operator/=(zero_type const& /*z*/) {
        return *this;
    }
};

constexpr zero_type zero = {};

template <typename T>
requires std::integral<no_cvr<T>> || std::floating_point<no_cvr<T>>
[[nodiscard]] constexpr auto operator<=>(zero_type /*z*/, T n) {
    return static_cast<T>(zero) <=> n;
}

template <typename T>
requires std::integral<no_cvr<T>> || std::floating_point<no_cvr<T>>
[[nodiscard]] constexpr auto operator<=>(T n, zero_type /*z*/) {
    return n <=> static_cast<T>(zero);
}

template <typename T>
requires std::integral<no_cvr<T>> || std::floating_point<no_cvr<T>>
[[nodiscard]] constexpr bool operator==(T n, zero_type /*z*/) {
    return n == static_cast<T>(zero);
}

template <typename T>
requires std::integral<no_cvr<T>> || std::floating_point<no_cvr<T>>
[[nodiscard]] constexpr bool operator==(zero_type /*z*/, T n) {
    return static_cast<T>(zero) == n;
}


}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_ZERO_HPP */
