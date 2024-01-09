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

#ifndef CAV_INCLUDE_UTILS_CALL_UTILS_HPP
#define CAV_INCLUDE_UTILS_CALL_UTILS_HPP

#include <type_traits>

#include "../comptime/mp_base.hpp"
#include "../comptime/syntactic_sugars.hpp"

/// @brief Wrap a function call within a lambda, useful since function-templates are second class
/// citizens and cannot be passed as template parameters (implementing everything in Circle-lang
/// would have been a good idea).
#define LAMBDA_WRAP(FN) [&](auto&&... args) { return FN(FWD(args)...); }

namespace cav {

/// @brief Sanititze lambda and get its return type
template <typename CallT, typename... Args>
struct lambda_ret {
    static_assert(std::is_invocable_v<no_cvr<CallT>, Args...>,
                  "Tried to invoke a lambda having incompatible arguments.");
    using ret_type = std::invoke_result_t<CallT, Args...>;
};

template <typename CallT, typename... Args>
using lambda_ret_t = typename lambda_ret<CallT, Args...>::ret_type;

/// @brief Some lambdas return a boolean to notify a possible early exit criterium (from a loop).
/// This wrapper eclose the needed checks always returning a boolean
[[nodiscard]] constexpr bool ret_bool_or_false(auto&& lambda, auto&&... args) {
    using lambda_res = std::invoke_result_t<TYPEOF(lambda), TYPEOF(args)...>;
    if constexpr (std::same_as<lambda_res, bool>)
        return FWD(lambda)(FWD(args)...);
    else
        FWD(lambda)(FWD(args)...);
    return false;
}

/// @brief Callable class/struct builder.
/// Given a function pointer, defines an equivalent callble struct
///
/// @tparam *F Function pointer
template <auto* F>
struct Ftor {};

template <class Ret, class... Args, auto (*F)(Args...)->Ret>
struct Ftor<F> {
    using return_type = Ret;
    using args_type   = pack<Args...>;

    constexpr Ret operator()(Args... args) {
        return F(FWD(args)...);
    }
};

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_CALL_UTILS_HPP */
