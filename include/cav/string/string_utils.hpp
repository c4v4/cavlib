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

#ifndef CAV_INCLUDE_STRINGUTILS_HPP
#define CAV_INCLUDE_STRINGUTILS_HPP

#include <charconv>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../comptime/type_name.hpp"

#if __has_include(<fmt/core.h>)
#define CAV_FOUND_FMT
#include <fmt/core.h>
#endif


#define SPACES " \t\n\r\f\v"

namespace cav {

constexpr std::string_view filename(std::string_view string) {
    int i = static_cast<int>(std::size(string)) - 1;
    while (i >= 0 && string[i] != '/')
        --i;
    return {std::begin(string) + i + 1, std::end(string)};
}

constexpr std::string_view remove_extension(std::string_view string) {
    int i = static_cast<int>(std::size(string)) - 1;
    while (i > 0 && string[i] != '.')
        --i;
    if (i > 0)
        return {std::begin(string), std::begin(string) + i};
    return string;
}

// Inplace with string
static inline void ltrim(std::string& s, char const* delim = SPACES) {
    auto pos = s.find_first_not_of(delim);
    if (pos != std::string::npos)
        s.erase(pos);
}

static inline void rtrim(std::string& s, char const* delim = SPACES) {
    auto pos = s.find_last_not_of(delim);
    if (pos != std::string::npos)
        s.erase(0, pos + 1);
}

inline void trim(std::string& s, char const* delim = SPACES) {
    ltrim(s, delim);
    rtrim(s, delim);
}

// Remove multiple adjacent chars listed in delim, leaving only delim[0] as delimiter char
static inline void remove_multiple_adj(std::string& s, std::string_view delim = SPACES) {

    auto s_wit         = s.begin();
    bool prev_is_delim = false;
    for (char& c : s) {
        if (std::ranges::find(delim, c) != delim.end()) {
            if (!prev_is_delim) {
                *s_wit = delim[0];
                ++s_wit;
                prev_is_delim = true;
            }
        } else {
            *s_wit = c;
            ++s_wit;
            prev_is_delim = false;
        }
    }
    s.erase(s_wit, s.end());
}

// Return "new" object with string_view (which are only a proxy on the same memory)
static inline std::string_view ltrim(std::string_view s, char const* delim = SPACES) {
    auto pos = s.find_first_not_of(delim);
    if (pos != std::string_view::npos)
        return s.substr(pos);
    return {};
}

static inline std::string_view rtrim(std::string_view s, char const* delim = SPACES) {
    auto pos = s.find_last_not_of(delim);
    if (pos != std::string_view::npos)
        return s.substr(0, pos + 1);
    return {};
}

static inline std::string_view trim(std::string_view s, char const* delim = SPACES) {
    return rtrim(ltrim(s, delim), delim);
}

inline std::string remove_multiple_adj(std::string_view& s, char const* delim = SPACES) {
    std::string s_new(s);
    remove_multiple_adj(s_new, delim);
    return s_new;
}

inline auto split_line(std::string_view line, std::string_view delim = SPACES) {
    std::vector<std::string_view> tokens;

    int lsize  = static_cast<int>(line.size());
    int last_i = 0;
    for (int i = 0; i < lsize; ++i) {
        if (std::ranges::find(delim, line[i]) != delim.end()) {
            if (i > last_i) {
                tokens.emplace_back(line.data() + last_i, i - last_i);
                tokens.back() = trim(tokens.back());
            }
            last_i = i + 1;
        }
    }
    if (last_i < lsize) {
        tokens.emplace_back(line.data() + last_i, lsize - last_i);
        tokens.back() = trim(tokens.back());
    }
    return tokens;
}

inline auto split_line(std::string_view line, char delim) {
    return split_line(line, std::string(1, delim));
}

template <typename T>
auto from_string_view(std::string_view s, T& val) {
    return std::from_chars(s.data(), s.data() + s.size(), val);
}

#ifdef CAV_FOUND_FMT
template <typename T, typename OnErrT = decltype([] { abort(); })>
void from_string_view_checked(std::string_view str, T& val, OnErrT&& on_error = {}) {
    auto res = std::from_chars(str.data(), str.data() + str.size(), val);
    if (res.ec == std::errc::invalid_argument) {
        fmt::print(stderr,
                   "Error: Unable to parse {} into variable of type {}.\n",
                   str,
                   static_cast<std::string_view>(type_name<T>::name));
        on_error();
    }
    if (res.ec == std::errc::result_out_of_range) {
        fmt::print(stderr,
                   "Error: Found value larger than type max ({} > {}).\n",
                   str,
                   static_cast<std::string_view>(type_name<T>::name));
        on_error();
    }
}

template <typename OnErrT = decltype([] { abort(); })>
constexpr void from_string_view_checked(std::string_view  str,
                                        std::string_view& val,
                                        OnErrT&& /*err*/ = {}) {
    val = str;
}

template <typename OnErrT = decltype([] { abort(); })>
constexpr void from_string_view_checked(std::string_view str,
                                        std::string&     val,
                                        OnErrT&& /*err*/ = {}) {
    val = str;
}

#endif

}  // namespace cav


#endif /* CAV_INCLUDE_STRINGUTILS_HPP */
