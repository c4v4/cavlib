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

#ifndef CAV_INCLUDE_UTILS_STATICSTR_HPP
#define CAV_INCLUDE_UTILS_STATICSTR_HPP

#include <array>
#include <cassert>
#include <string_view>

#include "mp_base.hpp"         //
#include "util_functions.hpp"  //

namespace cav {

/// @brief String literal made to be easily converted to other compile-time sized char containers
template <std::size_t N>
struct StaticStr : std::array<char, N> {
    using base = std::array<char, N>;
    using self = StaticStr;

private:
    static constexpr void _copy_init(auto& src, auto& dest) noexcept {
        // assert(src[N - 1] == '\0');
        for (std::size_t i = 0; i < N; ++i)
            dest[i] = src[i];
    }

public:
    constexpr StaticStr() = default;

    constexpr StaticStr(base const& str)
        : base{str} {
    }

    constexpr StaticStr(std::array<char const, N> const& str) {
        _copy_init(str, *this);
    }

    constexpr StaticStr(char const (&str)[N]) {
        _copy_init(str, *this);
    }

    constexpr StaticStr(char (&str)[N]) {
        _copy_init(str, *this);
    }

    constexpr StaticStr(std::string_view str) {
        size_t str_size = std::size(str);
        size_t sz       = str_size < N ? str_size : N - 1;

        for (size_t i = 0; i < sz; ++i)
            (*this)[i] = str[i];
        for (size_t i = sz; i < N; ++i)
            (*this)[i] = '\0';
    }

    constexpr operator std::array<char const, N>() const {
        std::array<char const, N> result{};
        _copy_init(*this, result);
        return result;
    }

    constexpr operator base const&() const {
        return *this;
    }

    constexpr operator base&() {
        return *this;
    }

    constexpr operator auto *() const {
        return base::data();
    }

    constexpr operator std::string_view() const {
        return {base::begin(), base::end() - 1};  // remove \0
    }

    constexpr operator fmt::string_view() const {
        return {base::data(), base::size() - 1};  // remove \0
    }

    [[nodiscard]] static constexpr std::size_t size() {
        return N;
    }
};

template <size_t M, size_t N>
constexpr auto operator<=>(StaticStr<M> const& s1, StaticStr<N> const& s2) noexcept {
    return std::string_view{s1} <=> std::string_view{s2};
}

template <size_t M, size_t N>
constexpr bool operator==(StaticStr<M> const& s1, StaticStr<N> const& s2) noexcept {
    return std::string_view{s1} == std::string_view{s2};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// COMPILE TIME FACILITIES /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//// Number of elements of a c-string or array-string
template <typename>
struct size_of;

template <size_t N>
struct size_of<StaticStr<N>> {
    static constexpr size_t value = N;
};

template <typename T, size_t N>
struct size_of<std::array<T, N>> {
    static constexpr size_t value = N;
};

template <typename T, size_t N>
struct size_of<T[N]> {
    static constexpr size_t value = N;
};

/// @brief Concatenate strings at compile time. Only c-array, std::array, and StaticStr can be
/// used (since the size must be known at compile time).
template <typename... Ts>
[[nodiscard]] constexpr auto str_concat(Ts&&... args) {
    // Compute new size removing zero-terminations
    constexpr size_t res_size = (size_of<no_cvr<Ts>>::value + ...) - sizeof...(Ts) + 1;

    auto result    = StaticStr<res_size>{};
    auto it        = result.begin();
    auto copy_into = [&](auto constr_str) {
        for (char c : constr_str) {
            assert(it < result.end());
            *it++ = c;
        }
        --it;
    };

    (copy_into(StaticStr(args)), ...);
    *it = '\0';
    assert(it + 1 == result.end());
    return result;
}

template <typename ST1, typename ST2>
requires requires(ST1 s1, ST2 s2) {
    { StaticStr(s1) };
    { StaticStr(s2) };
}
[[nodiscard]] constexpr decl_auto operator+(ST1&& s1, ST2&& s2) {
    return str_concat(StaticStr(FWD(s1)), StaticStr(FWD(s2)));
}

#ifndef NDEBUG
namespace test {
    static constexpr auto stra   = StaticStr{"a"};
    static constexpr char strb[] = "b";
    static constexpr auto strc   = std::array{'c', '\0'};

    static_assert(str_concat(stra) == StaticStr{"a"});
    static_assert(str_concat(strb) == StaticStr{"b"});
    static_assert(str_concat(strc) == StaticStr{"c"});
    static_assert(str_concat(stra, stra) == StaticStr{"aa"});
    static_assert(str_concat(strb, strb) == StaticStr{"bb"});
    static_assert(str_concat(strc, strc) == StaticStr{"cc"});
    static_assert(str_concat(stra, strc) == StaticStr{"ac"});
    static_assert(str_concat(strb, stra) == StaticStr{"ba"});
    static_assert(str_concat(strc, strb) == StaticStr{"cb"});
    static_assert(str_concat(stra, strb, strc) == StaticStr{"abc"});
}
#endif

template <std::integral auto Val>
[[nodiscard]] consteval auto int_to_const_str() {
    constexpr auto  size = ilog10(Val) + 2;
    StaticStr<size> result;

    auto n        = Val;
    result.back() = '\0';
    for (int i = size - 2; i >= 0; --i) {
        result[i] = '0' + (n % 10);
        n /= 10;
    }

    return result;
}

#ifndef NDEBUG
namespace test {
    static_assert(int_to_const_str<12345>()[0] == '1');
    static_assert(int_to_const_str<12345>()[1] == '2');
    static_assert(int_to_const_str<12345>()[2] == '3');
    static_assert(int_to_const_str<12345>()[3] == '4');
    static_assert(int_to_const_str<12345>()[4] == '5');
    static_assert(int_to_const_str<12345>()[5] == '\0');
}
#endif

}  // namespace cav

template <cav::StaticStr Str>
constexpr auto operator""_s() noexcept {
    return Str;
}

template <cav::StaticStr Str>
constexpr cav::ct<Str> operator""_cs() noexcept {
    return {};
}

static_assert(cav::StaticStr("prova") == "prova"_s);
static_assert(cav::eq<cav::StaticStr<6>, TYPEOF("prova"_s)>);

#endif /* CAV_INCLUDE_UTILS_STATICSTR_HPP */
