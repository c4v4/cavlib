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

#ifndef CAV_INCLUDE_UTILS_SCALEDINT_HPP
#define CAV_INCLUDE_UTILS_SCALEDINT_HPP

#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <type_traits>

#include "../numeric/zero.hpp"
#include "../string/StaticStr.hpp"

namespace cav {

// Tags, not enum because passed to ctor
struct round_tag {};

struct floor_tag {};

struct ceil_tag {};

struct trunc_tag {};

template <typename T>
concept rounding_tag = std::same_as<T, round_tag> || std::same_as<T, floor_tag> ||
                       std::same_as<T, ceil_tag> || std::same_as<T, trunc_tag>;

template <int8_t Exp, uint64_t Base = 10>
struct Pow {
    static_assert(pow_oveflow_check<uint64_t>(Base, abs(Exp)),
                  "Overflow detected with the given base and exponent.");
    static_assert(Base > 0, "Base must be >= 0.");

    static constexpr int8_t  exp_val  = Exp;
    static constexpr int64_t base_val = Base;
    static constexpr int64_t val      = pow<int64_t>(base_val, abs(exp_val));
};

template <int8_t        Exp,
          int64_t       Base          = 10,
          std::integral IntT          = int64_t,
          rounding_tag  DefaultRoundT = trunc_tag>
struct ScaledInt {
    static_assert(Base > 0, "The base of the scaling power must be positive");

    using int_type                 = IntT;
    using pow                      = Pow<Exp, Base>;
    using self                     = ScaledInt<Exp, Base, IntT, DefaultRoundT>;
    static constexpr auto exp_val  = Exp;
    static constexpr auto base_val = Base;

    int_type value;

private:
    struct private_tag {};

    constexpr ScaledInt(int_type val, private_tag&& /*unused*/) noexcept
        : value(val){};

public:
    ScaledInt() = default;

    template <typename T, rounding_tag TagT = DefaultRoundT>
    constexpr ScaledInt(T val, TagT&& /*unused*/ = {}) noexcept
        : value(_scale(val, TagT{})) {
    }

    /// @brief Same base conversion
    template <int8_t E, std::integral BT, rounding_tag TagT = DefaultRoundT>
    constexpr ScaledInt(ScaledInt<E, Base, BT> f, TagT&& /*unused*/ = {}) noexcept
        : value(_scale<int_type, Exp - E>(f.value, TagT{})) {
    }

    template <typename T>
    constexpr self& operator=(T val) noexcept {
        value = _scale(val, DefaultRoundT{});
        return *this;
    }

    template <typename T, rounding_tag TagT = DefaultRoundT>
    constexpr void from_val(T val, TagT&& /*unused*/ = {}) noexcept {
        value = _scale(val, TagT{});
    }

    template <int8_t E, std::integral BT, rounding_tag TagT = DefaultRoundT>
    constexpr void from_val(ScaledInt<E, Base, BT> f, TagT&& /*unused*/ = {}) noexcept {
        value = _scale<int_type, Exp - E>(f.value, TagT{});
    }

    [[nodiscard]] static constexpr self max_val() noexcept {
        return self(type_max<int_type>, private_tag{});
    }

    [[nodiscard]] static constexpr self min_val() noexcept {
        return self(type_min<int_type>, private_tag{});
    }

    [[nodiscard]] static constexpr self epsilon() noexcept {
        return self(1, private_tag{});
    }

    template <std::floating_point T>
    [[nodiscard]] explicit constexpr operator T() const {
        if constexpr (Exp >= 0)
            return value / T{pow::val};
        return value * T{pow::val};
    }

    template <std::integral T>
    [[nodiscard]] explicit constexpr operator T() const {
        static_assert(type_max<T> >= pow::val);
        if constexpr (Exp >= 0)
            return value / pow::val;
        return value * pow::val;
    }

    template <std::integral T, rounding_tag TagT = DefaultRoundT>
    [[nodiscard]] constexpr T to_integral(TagT&& /*unused*/ = {}) const {
        return _unscale(value, TagT{});
    }

    constexpr self& operator+=(self f) noexcept {
        value += f.value;
        return *this;
    }

    constexpr self& operator-=(self f) noexcept {
        value -= f.value;
        return *this;
    }

    [[nodiscard]] constexpr self& operator++() noexcept {
        ++value;
        return *this;
    }

    [[nodiscard]] constexpr self operator++(int) const noexcept {
        auto old = *this;
        ++value;
        return old;
    }

    template <typename T>
    constexpr self& operator*=(T n) noexcept {
        value *= n;
        return *this;
    }

    template <typename T>
    constexpr self& operator/=(T n) noexcept {
        value /= n;
        return *this;
    }

    [[nodiscard]] constexpr self& operator--() noexcept {
        --value;
        return *this;
    }

    [[nodiscard]] constexpr self operator--(int) const {
        auto old = *this;
        --value;
        return old;
    }

    [[nodiscard]] constexpr auto operator<=>(self other) const {
        return value <=> other.value;
    }

    template <int8_t E, std::integral BT>
    [[nodiscard]] constexpr auto operator<=>(ScaledInt<E, Base, BT> other) const {
        if constexpr (Exp < E)
            return other <=> *this;
        return value <=> _scale<int_type, Exp - E>(other.value);
    }

    template <typename T>
    [[nodiscard]] constexpr auto operator<=>(T n) const {
        return operator<=>(self(n));
    }

    [[nodiscard]] constexpr bool operator==(self other) const {
        return value == other.value;
    }

    template <int8_t E, std::integral BT>
    [[nodiscard]] constexpr bool operator==(ScaledInt<E, Base, BT> other) const {
        if constexpr (Exp < E)
            return other == *this;
        return value == _scale<int_type, Exp - E>(other.value);
    }

    template <typename T>
    [[nodiscard]] constexpr bool operator==(T n) const {
        return operator==(self{n});
    }

    [[nodiscard]] constexpr int_type get_base() const {
        return value;
    }

    template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
    friend struct ScaledInt;

private:
    template <typename T, rounding_tag TagT = DefaultRoundT>
    [[nodiscard]] static constexpr int_type _scale(T n, TagT&& /*unused*/ = {}) noexcept {
        return _scale<int_type, exp_val, base_val>(n, TagT{});
    }

    template <typename RetT,
              int8_t  E = Exp,
              int64_t B = Base,
              typename T,
              rounding_tag TagT = DefaultRoundT>
    [[nodiscard]] static constexpr RetT _scale(T n, TagT&& /*unused*/ = {}) noexcept {
        using pow_t = Pow<E, B>;
        if constexpr (E < 0)
            return _div(n, pow_t::val, TagT{});

        assert(static_cast<RetT>(n) <= static_cast<RetT>(type_max<RetT> / pow_t::val));
        return _mul(n, pow_t::val, TagT{});
    }

    template <typename T, rounding_tag TagT = DefaultRoundT>
    [[nodiscard]] static constexpr int_type _unscale(T n, TagT&& /*unused*/ = {}) noexcept {
        if constexpr (pow::exp_val < 0)
            return _mul(n, pow::val, TagT{});
        return _div(n, pow::val, TagT{});
    }

    template <rounding_tag TagT>
    [[nodiscard]] static constexpr int_type _to_int_type(std::floating_point auto f,
                                                         TagT&& /*unused*/) noexcept {
        auto int_f = static_cast<int_type>(f);
        using T    = TYPEOF(f);

        constexpr T half = T{0.5};
        if constexpr (std::same_as<TagT, round_tag>)
            return static_cast<int_type>(f + (f >= zero ? half : -half));
        else if constexpr (std::same_as<TagT, floor_tag>)
            return int_f - (f < int_f);
        else if constexpr (std::same_as<TagT, ceil_tag>)
            return int_f + (f > int_f);
        else {
            static_assert(std::same_as<TagT, trunc_tag>);
            return int_f;
        }
    }

    // MUL
    template <std::integral T1, std::integral T2, rounding_tag TagT>
    [[nodiscard]] static constexpr auto _mul(T1 n1, T2 n2, TagT&& /*unused*/) noexcept {
        return n1 * n2;
    }

    template <typename T1, typename T2, rounding_tag TagT>
    requires std::floating_point<T1> || std::floating_point<T2>
    [[nodiscard]] static constexpr auto _mul(T1 n1, T2 n2, TagT&& /*unused*/) noexcept {
        return _to_int_type(n1 * n2, TagT{});
    }

    // DIV

    template <std::integral T1, std::integral T2, rounding_tag TagT>
    [[nodiscard]] static constexpr auto _div(T1 n1, T2 n2, TagT&& /*unused*/) noexcept {

        if constexpr (std::same_as<TagT, round_tag>) {
            auto half_n2 = n2 / 2;
            if ((n1 < 0) != (n2 < 0))
                return (n1 - half_n2) / n2;
            return (n1 + half_n2) / n2;
        }

        auto res = n1 / n2;
        auto rem = n1 % n2;
        if constexpr (std::same_as<TagT, floor_tag>) {
            if ((n1 < 0) == (n2 < 0))
                return res;
            return res - (rem != 0);
        }
        if constexpr (std::same_as<TagT, ceil_tag>) {
            if ((n1 < 0) == (n2 < 0))
                return res + (rem != 0);
            return res;
        }
        if constexpr (std::same_as<TagT, trunc_tag>)
            return res;
    }

    template <typename T1, typename T2, rounding_tag TagT>
    requires std::floating_point<T1> || std::floating_point<T2>
    [[nodiscard]] static constexpr auto _div(T1 n1, T2 n2, TagT&& /*unused*/) noexcept {
        return _to_int_type(n1 / n2, TagT{});
    }
};

template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator+(ScaledInt<E, B, BT, RT> f1) noexcept {
    return f1;
}

template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator-(ScaledInt<E, B, BT, RT> f1) noexcept {
    return -1 * f1;
}

template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator+(ScaledInt<E, B, BT, RT> f1,
                                                          ScaledInt<E, B, BT, RT> f2) noexcept {
    return f1 += f2;
}

template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator-(ScaledInt<E, B, BT, RT> f1,
                                                          ScaledInt<E, B, BT, RT> f2) noexcept {
    return f1 -= f2;
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator+(T n, ScaledInt<E, B, BT, RT> f) noexcept {
    return f += ScaledInt<E, B, BT, RT>{n};
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator+(ScaledInt<E, B, BT, RT> f, T n) noexcept {
    return f += ScaledInt<E, B, BT, RT>{n};
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator-(T n, ScaledInt<E, B, BT, RT> f) noexcept {
    return f -= ScaledInt<E, B, BT, RT>{n};
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator-(ScaledInt<E, B, BT, RT> f, T n) noexcept {
    return f -= ScaledInt<E, B, BT, RT>{n};
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator*(T n, ScaledInt<E, B, BT, RT> f) noexcept {
    return f *= n;
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator*(ScaledInt<E, B, BT, RT> f, T n) noexcept {
    return f *= n;
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator/(T n, ScaledInt<E, B, BT, RT> f) noexcept {
    return f /= n;
}

template <typename T, int8_t E, int64_t B, std::integral BT, rounding_tag RT>
[[nodiscard]] constexpr ScaledInt<E, B, BT, RT> operator/(ScaledInt<E, B, BT, RT> f, T n) noexcept {
    return f /= n;
}

////////////// EXETEND LIMITS TO SUPPORT SCALEDINT //////////////
template <int8_t E, int64_t B, std::integral BT, rounding_tag RT>
struct limits<ScaledInt<E, B, BT, RT>> {
    using T = ScaledInt<E, B, BT, RT>;

    static constexpr T max = T::max_val();
    static constexpr T min = T::min_val();
};

namespace scaled_int_tests {
    // Small compile time test set 3.141592653589793238462643383279502884L
    static_assert(ScaledInt<-5>(std::numbers::pi * 100000) == ScaledInt<-5>(300000));
    static_assert(ScaledInt<-5>(std::numbers::pi * 100000) == 300000);
    static_assert(ScaledInt<-4>(std::numbers::pi * 100000) == 310000);
    static_assert(ScaledInt<-3>(std::numbers::pi * 100000) == 314000);
    static_assert(ScaledInt<-2>(std::numbers::pi * 100000) == 314100);
    static_assert(ScaledInt<-1>(std::numbers::pi * 100000) == 314150);
    static_assert(ScaledInt<-0>(std::numbers::pi * 100000) == 314159);
    static_assert(ScaledInt<+0>(std::numbers::pi * 100000) == 314159);
    static_assert(ScaledInt<+1>(std::numbers::pi * 100000) == 314159.2);
    static_assert(ScaledInt<+2>(std::numbers::pi * 100000) == 314159.26);
    static_assert(ScaledInt<+3>(std::numbers::pi * 100000) == 314159.265);
    static_assert(ScaledInt<+4>(std::numbers::pi * 100000) == 314159.2653);
    static_assert(ScaledInt<+5>(std::numbers::pi * 100000) == 314159.26535);

    static_assert(ScaledInt<0>(3.2) != ScaledInt<4>(3.2));                  // 3 != 3.2000
    static_assert(ScaledInt<1>(3.2) == ScaledInt<4>(3.2));                  // 3.2 == 3.2000
    static_assert(ScaledInt<4>(3.2) == ScaledInt<1>(3.2));                  // 3.2000 == 3.2
    static_assert(ScaledInt<2>(1234567.8945) == ScaledInt<9>(1234567.89));  // ..89 == ..890000000

    // Wrappers for standard types
    using Int32 = ScaledInt<0, 2, int32_t>;
    using Int64 = ScaledInt<0, 2, int64_t>;

    static_assert(type_max<Int32> == type_max<int32_t>);
    static_assert(type_min<Int32> == type_min<int32_t>);
    static_assert(type_max<Int64> == type_max<int64_t>);
    static_assert(type_min<Int64> == type_min<int64_t>);

    // Scaled signed integers with rounding and different precision
    using RInt32D1 = ScaledInt<1, 10, int32_t>;
    using RInt64D1 = ScaledInt<1, 10, int64_t>;
    using RInt32D2 = ScaledInt<2, 10, int32_t>;
    using RInt64D2 = ScaledInt<2, 10, int64_t>;
    using RInt32D3 = ScaledInt<3, 10, int32_t>;
    using RInt64D3 = ScaledInt<3, 10, int64_t>;

    static_assert(type_max<RInt32D1> > type_max<int32_t> / 10);
    static_assert(type_min<RInt32D1> < type_min<int32_t> / 10);
    static_assert(type_max<RInt32D2> > type_max<int32_t> / 100);
    static_assert(type_min<RInt32D2> < type_min<int32_t> / 100);
    static_assert(type_max<RInt32D3> > type_max<int32_t> / 1000);
    static_assert(type_min<RInt32D3> < type_min<int32_t> / 1000);

    static_assert(type_max<RInt64D1> > type_max<int64_t> / 10);
    static_assert(type_min<RInt64D1> < type_min<int64_t> / 10);
    static_assert(type_max<RInt64D2> > type_max<int64_t> / 100);
    static_assert(type_min<RInt64D2> < type_min<int64_t> / 100);
    static_assert(type_max<RInt64D3> > type_max<int64_t> / 1000);
    static_assert(type_min<RInt64D3> < type_min<int64_t> / 1000);
}  // namespace scaled_int_tests

// TODO(cava): move the default definition in another place
template <std::floating_point T, typename ArgT>
[[nodiscard]] constexpr T sround(ArgT n) noexcept {
    constexpr ArgT half = static_cast<ArgT>(0.5);
    return std::trunc(n + (n >= zero ? half : -half));
}

template <std::integral T, typename ArgT>
[[nodiscard]] constexpr T sround(ArgT n) noexcept {
    constexpr ArgT half = static_cast<ArgT>(0.5);
    return static_cast<T>(n + (n >= zero ? half : -half));
}

template <typename T>
[[nodiscard]] constexpr T sfloor(auto n) noexcept {
    return static_cast<T>(std::floor(n));
}

template <typename T>
[[nodiscard]] constexpr T sceil(auto n) noexcept {
    return static_cast<T>(std::ceil(n));
}

template <typename T>
[[nodiscard]] constexpr T strunc(auto n) noexcept {
    return static_cast<T>(std::trunc(n));
}

template <typename ScaledT>
[[nodiscard]] constexpr ScaledT sround(std::floating_point auto n) noexcept {
    return ScaledT(n, round_tag{});
}

template <typename ScaledT>
[[nodiscard]] constexpr ScaledT sfloor(std::floating_point auto n) noexcept {
    return ScaledT(n, floor_tag{});
}

template <typename ScaledT>
[[nodiscard]] constexpr ScaledT sceil(std::floating_point auto n) noexcept {
    return ScaledT(n, ceil_tag{});
}

template <typename ScaledT>
[[nodiscard]] constexpr ScaledT strunc(std::floating_point auto n) noexcept {
    return ScaledT(n, trunc_tag{});
}

template <std::integral IntT, typename ScaledT>
[[nodiscard]] constexpr IntT iround(ScaledT n) noexcept {
    return n.template to_integral<IntT>(round_tag{});
}

template <std::integral IntT, typename ScaledT>
[[nodiscard]] constexpr IntT ifloor(ScaledT n) noexcept {
    return n.template to_integral<IntT>(floor_tag{});
}

template <std::integral IntT, typename ScaledT>
[[nodiscard]] constexpr IntT iceil(ScaledT n) noexcept {
    return n.template to_integral<IntT>(ceil_tag{});
}

template <std::integral IntT, typename ScaledT>
[[nodiscard]] constexpr IntT itrunc(ScaledT n) noexcept {
    return n.template to_integral<IntT>(trunc_tag{});
}

namespace scaled_int_tests {
    static_assert(sround<ScaledInt<3>>(std::numbers::pi) == 3.142);
    static_assert(sfloor<ScaledInt<3>>(std::numbers::pi) == 3.141);
    static_assert(sceil<ScaledInt<3>>(std::numbers::pi) == 3.142);
    static_assert(strunc<ScaledInt<3>>(std::numbers::pi) == 3.141);
    static_assert(sround<ScaledInt<3>>(-std::numbers::pi) == -3.142);
    static_assert(sfloor<ScaledInt<3>>(-std::numbers::pi) == -3.142);
    static_assert(sceil<ScaledInt<3>>(-std::numbers::pi) == -3.141);
    static_assert(strunc<ScaledInt<3>>(-std::numbers::pi) == -3.141);

    static_assert(iround<int64_t>(ScaledInt<3>(std::numbers::pi * 1000)) == 3142);
    static_assert(ifloor<int32_t>(ScaledInt<3>(std::numbers::pi * 1000)) == 3141);
    static_assert(iceil<int64_t>(ScaledInt<3>(std::numbers::pi * 1000)) == 3142);
    static_assert(itrunc<int16_t>(ScaledInt<3>(std::numbers::pi * 1000)) == 3141);
    static_assert(iround<int64_t>(ScaledInt<3>(-std::numbers::pi * 1000)) == -3142);
    static_assert(ifloor<int32_t>(ScaledInt<3>(-std::numbers::pi * 1000)) == -3142);
    static_assert(iceil<int64_t>(ScaledInt<3>(-std::numbers::pi * 1000)) == -3141);
    static_assert(itrunc<int16_t>(ScaledInt<3>(-std::numbers::pi * 1000)) == -3141);

    static_assert(std::regular<ScaledInt<1>>);
    static_assert(std::totally_ordered<ScaledInt<1>>);
}  // namespace scaled_int_tests
}  // namespace cav


#if __has_include(<fmt/core.h>)
#include <fmt/core.h>

namespace fmt {
template <int8_t E, int64_t B, std::integral BT, cav::rounding_tag RT>
struct formatter<cav::ScaledInt<E, B, BT, RT>> : fmt::formatter<double> {
    auto format(cav::ScaledInt<E, B, BT, RT> c, format_context& ctx) noexcept {
        return fmt::formatter<double>::format(static_cast<double>(c), ctx);
    }
};
}  // namespace fmt
#endif

// namespace ska::detail {
// template <int8_t E, int64_t B, std::integral BT, cav::rounding_tag RT>
// inline int64_t to_radix_sort_key(cav::ScaledInt<E, B, BT, RT> const& s) noexcept {
//     return s.get_base();
// }
// }  // namespace ska::detail

#endif /* CAV_INCLUDE_UTILS_SCALEDINT_HPP */
