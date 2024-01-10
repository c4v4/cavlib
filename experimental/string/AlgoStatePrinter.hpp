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

#ifdef CAV_INCLUDE_UTILS_ALGOSTATEPRINTER_HPP
#define CAV_INCLUDE_UTILS_ALGOSTATEPRINTER_HPP

#include <fmt/core.h>

#include "StaticStr.hpp"
#include "limits.hpp"
#include "tuple.hpp"

namespace cav {

template <typename T, int D = type_max<int>>
struct delay_pair {
    using type                 = T;
    static constexpr int delay = D;
};

/// @brief Over-engineered state printer
template <StaticStr Header,
          int       HeaderPeriod,
          StaticStr FmtString,
          int       EmptyUpdatePeriod,
          typename... Ts>
class AlgoStatePrinter {
public:
    using tup_type                      = tuple<typename Ts::type...>;
    static constexpr auto header        = str_concat("\n", Header, "\n");
    static constexpr int  h_period      = HeaderPeriod;
    static constexpr int  update_period = EmptyUpdatePeriod;
    static constexpr auto fmt_string    = FmtString;

    AlgoStatePrinter() = default;

    AlgoStatePrinter(FILE* c_output)
        : output(c_output) {
    }

    AlgoStatePrinter(Ts const&... ts, FILE* c_output = stdout)
        : state{ts...}
        , output(c_output) {
    }

    template <typename... OTs>
    void print_state(typename Ts::type const&... ts, OTs&&... other) {
        if (--header_back_counter <= 0) {
            fmt::print(output, header.data());
            header_back_counter = h_period;
        }
        static constexpr auto ext_str = str_concat(fmt_string, (void(wrap<OTs>{}), "{}")..., "\n");
        fmt::print(output, ext_str.data(), ts..., FWD(other)...);
    }

    bool update(typename Ts::type const&... ts, auto&&... other) {
        ++time;
        if (_ready_to_print(ts...)) {
            state           = {ts...};
            last_print_time = time;
            print_state(ts..., FWD(other)...);
            return true;
        }
        return false;
    }


private:
    [[nodiscard]] constexpr bool _ready_to_print(
        typename Ts::type const&... new_ts) const noexcept {
        int64_t delay = time - last_print_time;
        return (delay >= update_period) || state.reduce([&](typename Ts::type const&... ts) {
            return ((new_ts != ts && delay >= Ts::delay) || ...);
        });
    }

    [[nodiscard]] constexpr auto _min(auto a1, auto... args) {
        auto min = a1;
        return ((min = min <= args ? min : args), ...);
    }

    FILE*    output              = stdout;
    tup_type state               = {};
    int64_t  time                = 0;
    int64_t  last_print_time     = -_min(Ts::delay...);
    int64_t  header_back_counter = 0;
};

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_ALGOSTATEPRINTER_HPP */
