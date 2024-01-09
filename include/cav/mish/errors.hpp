#ifndef CAV_INCLUDE_UTILS_NOEXCEPTIONS_HPP
#define CAV_INCLUDE_UTILS_NOEXCEPTIONS_HPP

#include <fmt/core.h>

#include <cassert>

#include "../comptime/test.hpp"

#if __cpp_exceptions
#define CAV_TRY      try
#define CAV_CATCH(X) catch (X)
#define CAV_THROW(E) throw E
#else
#include <fmt/core.h>

#define CAV_TRY      if constexpr (true)
#define CAV_CATCH(X) if constexpr (false)

template <typename Exception>
consteval void CAV_THROW(Exception e) noexcept {
    fmt::print(stderr, "An exception has occurred:\n");
    if constexpr (std::is_fundamental_v<Exception>)
        fmt::print(stderr, "Exception value: {}\n", e);
    else
        fmt::print(stderr, "{}\n", e.what());
    abort();
}
#endif

namespace cav {
template <typename... Ts>
[[noreturn]] static void throw_with_message(fmt::format_string<Ts...> fmt, Ts&&... args) {
    auto msg = fmt::format(fmt, FWD(args)...);
    CAV_THROW(std::runtime_error(msg));
}

template <typename... Ts>
[[noreturn]] void exit_with_message(::fmt::format_string<Ts...> fmt, Ts&&... args) {
    std::fflush(stdout);
    fmt::print(stderr, fmt, FWD(args)...);
    std::fflush(stderr);
    assert(!"Debug mode: syntetic fail to pause before exit.");
    exit(EXIT_FAILURE);
}

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_NOEXCEPTIONS_HPP */
