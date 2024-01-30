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

#ifndef CAV_INCLUDE_TYPE_NAME_HPP
#define CAV_INCLUDE_TYPE_NAME_HPP

#include <array>
#include <source_location>
#include <string_view>

#include "../comptime/macros.hpp"   //
#include "../string/StaticStr.hpp"  //

// The old way
// #if defined(__clang__) || defined(__GNUC__)
// #define CAV_FN_NAME __PRETTY_FUNCTION__;
// #elif defined(_MSC_VER)
// #define CAV_FN_NAME __FUNCSIG__;
// #else
// #error "Unsupported compiler"
// #endif

namespace cav {
// Based on https://stackoverflow.com/a/68139582

template <typename T>
struct type_name {
private:
    template <typename U>
    static constexpr std::string_view _wrapped_type_name() {
        return std::source_location::current().function_name();
    }

    template <typename U>
    static consteval auto _type_name() {
        constexpr auto len4_name = _wrapped_type_name<void>();
        constexpr auto len3_name = _wrapped_type_name<int>();
        constexpr auto u_name    = _wrapped_type_name<U>();

        CAV_PASS(len4_name != len3_name);
        CAV_PASS(len4_name.size() > len3_name.size());
        constexpr size_t prefix_len    = len4_name.find("void");
        constexpr size_t one_char_size = 1;  // len4_name.size() - len3_name.size();
        constexpr size_t wout_type_len = len4_name.size() - 4 * one_char_size;
        constexpr size_t target_len    = (u_name.size() - wout_type_len) / one_char_size;

        return StaticStr<target_len + 1>(u_name.substr(prefix_len, target_len));
    }

    template <auto CsName>
    static consteval auto _object_type_name() {
        constexpr std::string_view name_sv    = CsName;
        constexpr size_t           colons_pos = name_sv.rfind("::");
        if constexpr (colons_pos == std::string_view::npos)
            return CsName;
        else {
            constexpr size_t new_size = CsName.size() - colons_pos - 2;
            return StaticStr<new_size>(name_sv.substr(colons_pos + 2));
        }
    }

public:
    static constexpr auto full_name  = _type_name<T>();
    static constexpr auto name       = _type_name<std::remove_cvref_t<T>>();
    static constexpr auto local_name = _object_type_name<name>();
};

#define CAV_STD_NAME_SPEC(X)                                                \
    template <>                                                             \
    struct type_name<std::X> {                                              \
        static constexpr auto full_name  = str_concat("std::", CAV_STR(X)); \
        static constexpr auto name       = str_concat("std::", CAV_STR(X)); \
        static constexpr auto local_name = StaticStr{CAV_STR(X)};           \
    }

CAV_STD_NAME_SPEC(string_view);
CAV_STD_NAME_SPEC(string);
CAV_STD_NAME_SPEC(int8_t);
CAV_STD_NAME_SPEC(int16_t);
CAV_STD_NAME_SPEC(int32_t);
CAV_STD_NAME_SPEC(int64_t);
CAV_STD_NAME_SPEC(uint8_t);
CAV_STD_NAME_SPEC(uint16_t);
CAV_STD_NAME_SPEC(uint32_t);
CAV_STD_NAME_SPEC(uint64_t);


}  // namespace cav

#endif /* CAV_INCLUDE_TYPE_NAME_HPP */
