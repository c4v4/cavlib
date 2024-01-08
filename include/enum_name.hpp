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

#ifndef CAV_INCLUDE_UTILS_ENUM_NAME_HPP
#define CAV_INCLUDE_UTILS_ENUM_NAME_HPP

#include <bit>
#include <cstddef>
#include <source_location>
#include <string_view>
#include <type_traits>

#include "StaticStr.hpp"
#include "type_name.hpp"

namespace cav {

namespace detail {
    template <typename E, auto X>
    struct enum_name_impl {
    private:
        template <auto Y>
        static constexpr std::string_view _wrapped_enum_name() {
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

#ifndef NDEBUG
    namespace test {
        enum class TEST { A, B };

        enum TEST2 { A, B };

        static_assert(enum_name_impl<TEST, TEST::A>::name == "A");
        static_assert(enum_name_impl<TEST, TEST::B>::name == "B");
        static_assert(enum_name_impl<TEST2, A>::name == "A");
        static_assert(enum_name_impl<TEST2, B>::name == "B");
        static_assert(enum_name_impl<TEST, 5>::name == "TEST)5");  // illegal
        static_assert(enum_name_impl<TEST, 5>::name == "TEST)5");  // illegal

    }  // namespace test
#endif

    constexpr bool ends_with(std::string_view str, std::string_view suffix) {
        return str.size() >= suffix.size() &&
               0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    template <typename ET, std::underlying_type_t<ET> D>
    struct compute_enum_size {
        static constexpr auto built_name = type_name<ET>::local_name + ")" + int_to_const_str<D>();
        static constexpr size_t value    = [] {
            if constexpr (ends_with(enum_name_impl<ET, D>::name, built_name))
                return D;
            else
                return compute_enum_size<ET, D + 1>::value;
        }();
    };
}  // namespace detail

/// @brief Create a map from enum values to their names
template <typename ET>
constexpr auto make_enum_name_map() {
    constexpr size_t e_size = detail::compute_enum_size<ET, 0>::value;
    auto             res    = std::array<char const*, e_size>{};
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((res[Is] = detail::enum_name_impl<ET, Is>::cs_name.data()), ...);
    }(std::make_index_sequence<e_size>{});
    return res;
}

template <auto X>
[[nodiscard]] constexpr auto enum_name() {
    return detail::enum_name_impl<decltype(X), X>::name;
}

[[nodiscard]] constexpr auto enum_name(auto x) {
    using enum_type    = decltype(x);
    constexpr auto map = make_enum_name_map<enum_type>();
    auto const     idx = static_cast<std::underlying_type_t<enum_type>>(x);
    return map[idx];
}

#ifndef NDEBUG
namespace detail::test {
    static_assert(enum_name<TEST::A>() == enum_name(TEST::A));
    static_assert(enum_name<TEST::B>() == enum_name(TEST::B));
    static_assert(enum_name<A>() == enum_name(A));
    static_assert(enum_name<B>() == enum_name(B));
}  // namespace detail::test
#endif

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_ENUM_NAME_HPP */
