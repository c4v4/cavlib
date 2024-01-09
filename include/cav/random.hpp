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

#ifndef CAV_INCLUDE_RANDOM_HPP
#define CAV_INCLUDE_RANDOM_HPP

#include <cassert>
#include <cmath>
#include <limits>

#include "XoshiroCpp.hpp"  //
#include "macros.hpp"
#include "mp_base.hpp"
#include "util_functions.hpp"

namespace cav {

using prng_int32_t  = XoshiroCpp::Xoshiro128PlusPlus;  // generic
using prng_int64_t  = XoshiroCpp::Xoshiro256PlusPlus;  // generic
using prng_float_t  = XoshiroCpp::Xoshiro128Plus;      // specialized for real distributions
using prng_double_t = XoshiroCpp::Xoshiro256Plus;      // specialized for real distributions

using prng_t = prng_int64_t;  // default

template <typename TpT = ct<0.5>>
[[nodiscard]] CAV_NOINLINE inline bool coin_flip(prng_t& rnd, auto true_prob = {}) noexcept {
    assert(0 <= true_prob && true_prob <= 1);
    using result_type = typename prng_t::result_type;
    auto treshold_val = static_cast<result_type>(static_cast<double>(true_prob) *
                                                 static_cast<double>(prng_t::max()));
    return rnd() <= treshold_val;
}

/// @brief Two coins flip mapped into one interval
/// Let's imagine the [0,1] interval.
/// We want with prob P to get 1 for both coin flips
/// We have 4 scenario with the following probabilities:
/// - toss 1 true: P
/// - toss 2 true: P
/// - toss 1&2 true: P^2
/// - toss 1&2 false: 1 - 2P + P^2
///
/// Which can be arranged as:
/// [0------------------------------------------1]
///  |------ P ------|
///        |------ P ------|
///        |-- P^2 --|     |-- (1 - 2P + P^2) --|
///
/// Splitting the [0,1] range into 4 subranges with the proper sizes.
///
[[nodiscard]] inline std::array<bool, 2> two_coin_flips(prng_t& rnd, auto true_p) noexcept {

    assert(0 <= true_p && true_p <= 1);
    using res_type                    = typename prng_t::result_type;
    static constexpr res_type res_max = prng_t::max();

    long double p           = static_cast<long double>(true_p);
    res_type    range_width = static_cast<res_type>(p * res_max);

    // First interval
    // static constexpr res_type beg1 = zero;
    res_type end1 = range_width;

    // Second interval
    res_type beg2 = static_cast<res_type>((p - p * p) * res_max);
    res_type end2 = beg2 + range_width;

    res_type rnd_val = rnd();
    return {rnd_val <= end1, beg2 <= rnd_val && rnd_val <= end2};
}

[[nodiscard]] inline double rnd_real(prng_t& rnd, auto min, auto max) noexcept {
    static constexpr double rnd_range = static_cast<double>(prng_t::max() - prng_t::min());
    double                  scale     = (max - min) / rnd_range;
    return min + scale * static_cast<double>(rnd());
}

[[nodiscard]] inline int roll_dice(prng_t& rnd, auto min, auto max) noexcept {
    assert(max - min >= 0);
    int result = min + static_cast<int>(rnd() % (max - min + 1));
    assert(min <= result && result <= max);
    return result;
}

}  // namespace cav

#endif /* CAV_INCLUDE_RANDOM_HPP */
