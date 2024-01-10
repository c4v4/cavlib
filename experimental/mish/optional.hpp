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

#ifndef CAV_INCLUDE_UTILS_OPTIONAL_HPP
#define CAV_INCLUDE_UTILS_OPTIONAL_HPP

#include <concepts>
#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "../comptime/call_utils.hpp"
#include "../comptime/macros.hpp"
#include "../comptime/mp_base.hpp"

/// @brief std::optional does not support void or references. Which is a bummer for templated stuff
/// Here some trivial extensions are implemented just to cover those cases.
namespace cav {

struct optional_void {
    constexpr optional_void(void_type /*v*/)
        : has_val(true){};

    constexpr optional_void()
        : has_val(false){};

    constexpr optional_void(std::nullopt_t /*v*/)
        : has_val(false){};

    constexpr void operator*() const noexcept {
    }

    constexpr void value() const noexcept {
    }

    constexpr void value_or(auto&& /*v*/) const noexcept {
    }

    constexpr operator bool() const noexcept {
        return has_val;
    }

    [[nodiscard]] constexpr bool has_value() const {
        return has_val;
    }

private:
    bool has_val;
};

template <typename T>
struct optional_reference {
    constexpr optional_reference(T&& arg)
        : val(std::addressof(arg)) {
    }

    constexpr optional_reference()
        : val(nullptr){};

    constexpr optional_reference(std::nullopt_t /*unused*/)
        : val(nullptr){};

    constexpr T const& operator*() const& noexcept {
        return *val;
    }

    constexpr T& operator*() & noexcept {
        return *val;
    }

    constexpr T&& operator*() && noexcept {
        return std::move(*val);
    }

    constexpr T const&& operator*() const&& noexcept {
        return std::move(*val);
    }

    constexpr T const* operator->() const noexcept {
        return val;
    }

    constexpr T* operator->() noexcept {
        return val;
    }

    constexpr T const& value() const& {
        if (val)
            return *val;
    }

    constexpr T& value() & {
        if (val)
            return *val;
    }

    constexpr T&& value() && {
        if (val)
            return std::move(*val);
    }

    constexpr T const&& value() const&& {
        if (val)
            return std::move(*val);
    }

    template <typename U>
    constexpr T value_or(U&& u) const& {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        if (val)
            return *val;
        return static_cast<T>(FWD(u));
    }

    template <typename U>
    constexpr T value_or(U&& u) && {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_convertible_v<U&&, T>);

        if (val)
            return std::move(*val);
        return static_cast<T>(FWD(u));
    }

    template <typename... Args>
    constexpr T& emplace(T& t) noexcept {
        val = std::addressof(t);
        return *val;
    }

    constexpr void reset() noexcept {
        val = nullptr;
    }

    constexpr void swap(optional_reference& other) noexcept {
        std::swap(val, other.val);
    }

private:
    T* val;
};

template <typename T>
struct pick_optional {
    using type                 = std::optional<T>;
    static constexpr type fail = type{std::nullopt};
};

template <>
struct pick_optional<void> {
    using type                 = optional_void;
    static constexpr type fail = type{std::nullopt};
};

template <>
struct pick_optional<void_type> {
    using type                 = optional_void;
    static constexpr type fail = type{std::nullopt};
};

template <typename T>
struct pick_optional<T&> {
    using type                 = optional_reference<T>;
    static constexpr type fail = type{std::nullopt};
};

template <typename T>
using pick_optional_t = typename pick_optional<T>::type;

template <typename T>
constexpr auto pick_optional_f = pick_optional<T>::fail;

/// @brief  Void cannot be passed as function parameter (but returning it is ok, wtf?).
/// Map a void returning invokable object to a void_type returning one.
template <typename OpT, typename... Args>
CAV_INLINE auto invoke_to_optional(OpT&& op, Args&&... args) {
    using ret_type = lambda_ret_t<OpT, Args...>;
    using opt_type = pick_optional_t<ret_type>;

    if constexpr (!std::same_as<ret_type, void>)
        return opt_type{FWD(op)(FWD(args)...)};
    else {
        FWD(op)(FWD(args)...);
        return opt_type{void_type{}};
    }
}

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_OPTIONAL_HPP */
