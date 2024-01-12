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

#ifndef CAV_INCLUDE_SYNTACTIC_SUGARS_HPP
#define CAV_INCLUDE_SYNTACTIC_SUGARS_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace cav {

/// @brief Ok, it's not really "best-practice", but I find myself writing too often shenanigans just
/// because perfect forwarding has an horrible syntax that requires the type explicitly. Even worse,
/// sometime I avoid perfect forwarding even if it would be the correct choice, just because the
/// small overhead is still preferable to the amount of boilerplate this things adds everywhere.
#define FWD(X) static_cast<decltype(X)&&>(X)  //::std::forward<decltype(X)>(X)

/// @brief Syntactic sugar to simplify the reading of function signatures. I know, it's a minor
/// thing that introduces yet another custom-symbol. However, when reading a function signature I
/// find more confusing having parentesis in the return type, since it adds another level of
/// complexity for something that, in the end, has the same role of "auto".
#define decl_auto decltype(auto)

#define UNIQUE_TYPE decltype([] {})

#define REQUIRES_FN(...) \
    requires {           \
        { __VA_ARGS__ }; \
    }

#define REQUIRES_TYPE(...) requires { typename __VA_ARGS__; }

constexpr auto nop = [](auto&&...) {};

/// @brief Shorthand for std::remove_cvref_t<T>;
template <typename T>
using no_cvr = std::remove_cvref_t<T>;

template <typename T1, typename T2>
concept eq = std::same_as<T1, T2>;

template <typename T1, typename T2>
concept eq_no_cvr = eq<no_cvr<T1>, T2>;

/// @brief Return the cleaned type of X. Only a shorthand to reduce boilerplate.
/// Why a macro?
/// - The result is a type so a function is not appropriate in this case.
/// - With class templates, we cannot pass non-constexpr objects (even though we only need
/// the type, which is a constexpr information).
#define TYPEOF(X) ::cav::no_cvr<decltype(X)>

/// @brief Yea, yea I know, redefining components of the language with other names is bad practice,
/// but come on, why everything must be so much verbose and uselessly boilerplately?
template <bool Cond, typename T1, typename T2>
using if_t = std::conditional_t<Cond, T1, T2>;

[[noreturn]] inline void unreachable() {
#ifdef __GNUC__  // GCC, Clang, ICC
    __builtin_unreachable();
#elif defined _MSC_VER  // MSVC
    __assume(false);
#endif
}

}  // namespace cav

#endif /* CAV_INCLUDE_SYNTACTIC_SUGARS_HPP */
