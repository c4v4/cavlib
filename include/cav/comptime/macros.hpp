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

#ifndef CAV_INCLUDE_MACROS_HPP
#define CAV_INCLUDE_MACROS_HPP


#ifdef NDEBUG
#define CAV_DEBUG if constexpr (false)
#else
#define CAV_DEBUG
#endif

#ifndef NDEBUG
#define VERBOSE
#endif

/// @brief Activates more logs
#ifdef VERBOSE
#define CAV_VERBOSE
#else
#define CAV_VERBOSE if constexpr (false)
#endif

/// @brief Always inline the function
#define CAV_INLINE [[gnu::always_inline]]  // __attribute__((__always_inline__)) inline

/// @brief Always inline the function
#define CAV_NOINLINE [[gnu::noinline]]

/// @brief Always inline the functions called in the specified function (clang and gcc differ!)
#define CAV_FLATTEN [[gnu::flatten]]  // __attribute__((flatten))
// #define CAV_FLATTEN [[gnu::noinline]] // __attribute__((noinline))

/// @brief Functions that have no observable effects on the state of the program other than to
/// return a value
#define CAV_PURE [[gnu::pure]]

/// @brief Function whose return value is not affected by changes to the observable state of the
/// program and are pure.
#define CAV_CONST [[gnu::const]]

/// @brief Inform the compiler that the function is unlikely to be executed
#define CAV_COLD [[gnu::cold]]

/// @brief Inform the compiler that the function is a hot spot of the compiled program
#define CAV_HOT [[gnu::hot]]


/// @brief Stringfy an expression
#define CAV_STR(s)  CAV__STR(s)
#define CAV__STR(s) #s

/// @brief Often one would like to use a type only if it is defined. However, std::conditional_t
/// instantiates both branches so one cannot insert non-existing types to check if they exist
/// with a require expression, and then use them safely.
/// On the other hand, "if constexpr" have exactly the desired behaviour (it does not instantiate
/// the non-used branch). So using a lambda that wraps the if constexpr we can get the check that we
/// want.
/// Void is returned if the request type does not exist.
#define CAV_VOID_IF_ILLEGAL(TYPE...)            \
    typename std::invoke_result_t<decltype([] { \
        if constexpr (requires { TYPE; })       \
            return wrap<TYPE>{};                \
        else                                    \
            return wrap<void>{};                \
    })>::type

#define CAV_FALLBK_TYPE(FALLBACK, TYPE...)         \
    typename std::invoke_result_t<decltype([] {    \
        if constexpr (requires { typename TYPE; }) \
            return wrap<typename TYPE>{};          \
        else                                       \
            return wrap<FALLBACK>{};               \
    })>::type

#define CAV_FALLBK_VALUE(FALLBACK, VALUE...) \
    [] {                                     \
        if constexpr (requires { VALUE; })   \
            return VALUE;                    \
        else                                 \
            return FALLBACK;                 \
    }()

#endif /* CAV_INCLUDE_MACROS_HPP */
