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

#ifndef CAV_INCLUDE_ENUM_NAME_HPP
#define CAV_INCLUDE_ENUM_NAME_HPP

#include <bit>
#include <cstddef>
#include <source_location>
#include <string_view>
#include <type_traits>

#include "../comptime/type_name.hpp"
#include "../string/StaticStr.hpp"

namespace cav {

namespace detail {
    template <typename E, auto X>
    struct enum_name_impl {
    private:
        template <auto Y>
        static consteval std::string_view _wrapped_enum_name() {
            return std::source_location::current().function_name();
        }

        static consteval auto _enum_name() {
            constexpr int  iconst    = 134851047;  // To not match anything else
            constexpr auto sconst    = "134851047";
            constexpr auto len9_name = _wrapped_enum_name<iconst>();
            constexpr auto y_name    = _wrapped_enum_name<std::bit_cast<E>(
                static_cast<std::underlying_type_t<E>>(X))>();

            constexpr size_t prefix_len    = len9_name.find(sconst);
            constexpr size_t wout_type_len = len9_name.size() - 9;
            constexpr size_t target_len    = (y_name.size() - wout_type_len);

            // Use type_name to get the enum name (for consistency old enums)
            constexpr auto name_sv = y_name.substr(prefix_len, target_len);
            constexpr auto name_cs = StaticStr<target_len + 1>(name_sv);
            return name_cs;
        }

        template <auto CsName>
        static consteval auto _short_enum_name() {
            constexpr std::string_view local_name = CsName;
            constexpr size_t           colons_pos = local_name.rfind("::");
            if constexpr (colons_pos == std::string_view::npos)
                return CsName;
            else {
                constexpr size_t new_size = CsName.size() - colons_pos - 2;
                return StaticStr<new_size>(local_name.substr(colons_pos + 2));
            }
        }

    public:
        static constexpr auto             cs_name = _short_enum_name<_enum_name()>();
        static constexpr std::string_view name    = cs_name;
    };

#ifdef CAV_COMP_TESTS
    namespace {
        enum class TEST { A, B };

        enum TEST2 { A, B };

        CAV_PASS(enum_name_impl<TEST, TEST::A>::name == "A");
        CAV_PASS(enum_name_impl<TEST, TEST::B>::name == "B");
        CAV_PASS(enum_name_impl<TEST2, A>::name == "A");
        CAV_PASS(enum_name_impl<TEST2, B>::name == "B");
        CAV_PASS(enum_name_impl<TEST, 5>::name == "TEST)5");  // illegal
        CAV_PASS(enum_name_impl<TEST, 5>::name == "TEST)5");  // illegal

    }  // namespace
#endif

}  // namespace detail

template <typename ET, std::underlying_type_t<ET> D = 0>
struct enum_size {
    using utype                       = std::underlying_type_t<ET>;
    static constexpr auto  built_name = type_name<ET>::local_name + ")" + int_to_const_str<D>();
    static constexpr utype value      = [] {
        if constexpr (detail::enum_name_impl<ET, D>::name.ends_with(
                          static_cast<std::string_view>(built_name)))
            return D;
        else
            return enum_size<ET, D + 1>::value;
    }();
};

/// @brief Create a map from enum values to their names
template <typename ET>
consteval auto make_enum_name_map() {
    constexpr size_t e_size = enum_size<ET>::value;
    auto             res    = std::array<char const*, e_size>{};
    for_each_idx<e_size>(
        [&]<auto I>(ct<I>) { res[I] = detail::enum_name_impl<ET, I>::cs_name.data(); });
    return res;
}

template <auto X>
[[nodiscard]] consteval auto enum_name() {
    return detail::enum_name_impl<decltype(X), X>::name;
}

[[nodiscard]] constexpr auto enum_name(auto x) {
    using enum_type    = decltype(x);
    constexpr auto map = make_enum_name_map<enum_type>();
    auto const     idx = static_cast<std::underlying_type_t<enum_type>>(x);
    return map[idx];
}

#ifdef CAV_COMP_TESTS
namespace detail::test {
    CAV_PASS(enum_name<TEST::A>() == enum_name(TEST::A));
    CAV_PASS(enum_name<TEST::B>() == enum_name(TEST::B));
    CAV_PASS(enum_name<A>() == enum_name(static_cast<TEST>(0)));
    CAV_PASS(enum_name<B>() == enum_name(B));
}  // namespace detail::test
#endif

}  // namespace cav

#endif /* CAV_INCLUDE_ENUM_NAME_HPP */
