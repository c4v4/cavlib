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

#ifndef CAV_INCLUDE_MP_BASE_HPP
#define CAV_INCLUDE_MP_BASE_HPP

#include <cmath>
#include <concepts>
#include <type_traits>

#include "../comptime/syntactic_sugars.hpp"

template <char... Cs>
[[nodiscard]] constexpr std::size_t operator""_uz() {
    return [] {
        std::size_t n = 0;
        return ((n = n * 10 + (Cs - '0')), ..., n);
    }();
}

namespace cav {

/// @brief Void can be returned from auto functions, but cannot be passed as argument to other
/// functions (like ctors) with 0 arguments. E.g., wrapping the result of a lambda with a type
/// requires some special handling. This empty struct is used to represent void in this context.
/// (Why tho C++? What harm could have it done :/)
struct void_type {};

/// @brief Encloses a parameter pack. The basic building block to work with variadic templates.
template <typename... Ts>
struct pack {};

/// @brief Empty class template that wraps only wraps a type. Can be used when a type need to be
/// passed as function argument (e.g., as a tag), since wrap<T> is constexpr and default
/// constructible, while the T migh not be.
template <typename T>
struct wrap {
    using type = T;
};

template <typename T>
constexpr wrap<T> wrap_v = {};

/// @brief Like wrap, but every instantiation is different from the other ones.
template <typename T, typename = UNIQUE_TYPE>
struct unique_wrap : wrap<T> {};

template <typename T>
struct value_wrap : T {
    using type = T;

    constexpr value_wrap(T const& val)
        : T(val){};
};

// Wrapper needed for floating points with clang
template <typename T>
requires(!std::is_pointer_v<T> && !std::is_class_v<T>)
struct value_wrap<T> {
    using type = T;
    T value    = {};

    constexpr value_wrap(T val)
        : value(val){};

    [[nodiscard]] constexpr operator T const&() const {
        return value;
    }
};

template <typename T>
value_wrap(T val) -> value_wrap<no_cvr<T>>;

// And for compile-time c arrays
template <typename T, auto N>
struct value_wrap<T[N]> {
    using type = T const (&)[N];
    T value[N] = {};

    constexpr value_wrap(T const (&val)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            value[i] = val[i];
    };

    [[nodiscard]] constexpr operator type() const {
        return value;
    }

    [[nodiscard]] constexpr operator T const*() const {
        return value;
    }
};

template <typename T, auto N>
value_wrap(T const (&)[N]) -> value_wrap<T[N]>;

template <value_wrap X>
struct ct {
    using type                  = typename TYPEOF(X)::type;
    static constexpr type value = static_cast<type>(X);

    [[nodiscard]] constexpr operator type() const noexcept {
        return value;
    }
};

template <value_wrap X>
static constexpr auto ct_v = ct<X>{};

template <char... Cs>
[[nodiscard]] constexpr auto operator""_ct() {
    return ct_v<operator""_uz<Cs...>()>;
}

/// @brief Simple struct that inherits from all its template parameters
template <typename... Ts>
struct inherit : public Ts... {};

template <typename... Ts>
inherit(Ts...) -> inherit<Ts...>;

/// @brief Like inherit, but for lambdas, so that operator() is inherited from all the lambdas
template <typename... Ts>
struct multi_lambda : public Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
multi_lambda(Ts...) -> multi_lambda<Ts...>;

/// @brief False value that can be used in CAV_PASSs.
template <typename...>
constexpr bool always_false = false;

}  // namespace cav

#endif /* CAV_INCLUDE_MP_BASE_HPP */
