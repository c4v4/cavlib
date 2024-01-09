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

#ifndef CAV_INCLUDE_LIMITS_HPP
#define CAV_INCLUDE_LIMITS_HPP

#include <limits>

namespace cav {
///////// NUMERIC MAX AND MIN /////////

/// @brief Syntactic sugar for numeric max and min values.
///  It also extend the interface for tuple-like classes.
template <typename T>
struct limits {
    static constexpr T max = std::numeric_limits<T>::max();
    static constexpr T min = std::numeric_limits<T>::lowest();
};

template <template <typename...> class Tmpl, typename... Ts>
struct limits<Tmpl<Ts...>> {
    using T = Tmpl<Ts...>;

    static constexpr T max = T{std::numeric_limits<Ts>::max()...};
    static constexpr T min = T{std::numeric_limits<Ts>::lowest()...};
};

template <typename T>
constexpr T type_max = limits<T>::max;

template <typename T>
constexpr T type_min = limits<T>::min;

}  // namespace cav

#endif /* CAV_INCLUDE_LIMITS_HPP */
