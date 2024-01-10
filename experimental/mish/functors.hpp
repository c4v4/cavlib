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

#ifndef CAV_INCLUDE_UTILS_FUNCTORS_HPP
#define CAV_INCLUDE_UTILS_FUNCTORS_HPP


#include <cstdint>
#include <type_traits>
#include <utility>

#include "../comptime/syntactic_sugars.hpp"

namespace cav {

struct identity_ftor {
    template <typename T>
    constexpr T&& operator()(T&& t) const noexcept {
        return FWD(t);
    }
};

template <auto FieldT>
struct as_field;

template <typename Struct, typename FiledT, FiledT Struct::*Field>
struct as_field<Field> {
    auto& operator()(Struct& t) const {
        return t.*Field;
    }
};

template <typename T>
struct inc {
    constexpr T operator()(T t) const {
        return ++t;
    }
};

template <typename T>
struct dec {
    constexpr T operator()(T t) const {
        return --t;
    }
};

template <typename T1, typename T2>
struct plus {
    constexpr T1 operator()(T1 t, T2 i) const {
        return t + i;
    }
};

template <typename T1, typename T2>
struct minus {
    constexpr int64_t operator()(T1 t1, T2 t2) const {
        return t1 - t2;
    }
};

template <typename T>
struct defer {
    constexpr decl_auto operator()(T& t) const {
        return *t;
    }
};

struct less_ftor {
    constexpr bool operator()(auto const& a, auto const& b) const {
        return a < b;
    }
};

struct greater_ftor {
    constexpr bool operator()(auto const& a, auto const& b) const {
        return a > b;
    }
};

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_FUNCTORS_HPP */
