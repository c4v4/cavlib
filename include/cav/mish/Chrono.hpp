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

#ifndef CAV_INCLUDE_CAV_CHRONO_HPP
#define CAV_INCLUDE_CAV_CHRONO_HPP

#ifndef CAV_INCLUDEs_CHRONO_HPP
#define CAV_INCLUDEs_CHRONO_HPP

#include <chrono>

#include "../comptime/test.hpp"

namespace cav {

using nsec    = std::chrono::nanoseconds;
using usec    = std::chrono::microseconds;
using msec    = std::chrono::milliseconds;
using sec     = std::chrono::seconds;
using minutes = std::chrono::minutes;
using hours   = std::chrono::hours;
using days    = std::chrono::days;
using weeks   = std::chrono::weeks;
using months  = std::chrono::months;
using years   = std::chrono::years;

template <typename UnitT = usec>
struct Chrono {
    using high_res_clock = std::chrono::high_resolution_clock;
    using time_point     = std::chrono::system_clock::time_point;

    time_point start;
    time_point last;

    Chrono()
        : start(high_res_clock::now())
        , last(start) {
    }

    template <typename UnitT2>
    [[nodiscard]] static constexpr double time_cast(double t) {
        using factor = std::ratio_divide<typename UnitT::period, typename UnitT2::period>;
        return t * factor::num / factor::den;
    }

    void restart() {
        start = last = high_res_clock::now();
    }

    uint64_t lap() {
        auto old_last = last;
        last          = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(last - old_last).count();
    }

    [[nodiscard]] uint64_t from_start() const {
        auto now = high_res_clock::now();
        return std::chrono::duration_cast<UnitT>(now - start).count();
    }

    template <typename UnitT2>
    [[nodiscard]] auto from_start() const {
        return time_cast<UnitT2>(from_start());
    }

    template <typename UnitT2>
    [[nodiscard]] auto lap() {
        return time_cast<UnitT2>(lap());
    }
};

#ifdef CAV_COMP_TESTS
namespace {
    CAV_PASS(Chrono<nsec>::template time_cast<usec>(1) == 1e-3);
    CAV_PASS(Chrono<usec>::template time_cast<usec>(1) == 1.0);
    CAV_PASS(Chrono<msec>::template time_cast<usec>(1) == 1000.0);
    CAV_PASS(Chrono<sec>::template time_cast<usec>(1) == 1000000.0);
    CAV_PASS(Chrono<sec>::template time_cast<days>(86400) == 1.0);
    CAV_PASS(Chrono<days>::template time_cast<sec>(1) == 86400.0);
}
#endif

}  // namespace cav

#endif /* CAV_INCLUDE_CHRONO_HPP */


#endif /* CAV_INCLUDE_CAV_CHRONO_HPP */
