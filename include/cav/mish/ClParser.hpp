// Copyright (c) 2024 Francesco Cavaliere
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

#ifndef CAV_CAVLIB_INCLUDE_CAV_MISH_CLARGS_HPP
#define CAV_CAVLIB_INCLUDE_CAV_MISH_CLARGS_HPP

#include <fmt/core.h>

#include "cav/comptime/mp_base.hpp"
#include "cav/comptime/type_name.hpp"
#include "cav/mish/errors.hpp"
#include "cav/mish/util_functions.hpp"
#include "cav/string/StaticStr.hpp"
#include "cav/string/string_utils.hpp"
#include "cav/tuplish/type_map.hpp"

namespace cav {

static constexpr char spaces[] = " \t\n\v\f\r";

/// @brief A command-line argument parser that uses a type_map to manage values.
/// It parses command-line arguments into a map of key-value pairs, where the keys are the first
/// flag of the arguments and the values are the parsed argument values. It supports both positional
/// and flag-based arguments. It also provides a method to print the arguments and their
/// descriptions, which can be used for generating help messages
/// @tparam ArTs The types of the arguments.
template <typename... ArTs>
struct ClParser : type_map<ArTs...> {
    using base = type_map<ArTs...>;

    constexpr ClParser() = default;

    template <typename T>
    constexpr ClParser(std::span<T> const& args) {
        parse_cli(args);
    }

    template <typename T>
    constexpr void parse_cli(std::span<T> const& args) {
        for (auto it = args.begin() + 1; it != args.end();)
            if (!base::for_each([&](auto& arg) { return arg.try_consume(it); }))
                exit_with_message("Error: unknown argument: {}\n", *it);
    }

    template <size_t FlagSz = 0, size_t DescrSz = 24>
    void print_args() const {
        base::reduce([]<class... Ts>(Ts const&... args) {  // comptime formatting
            static constexpr size_t flag_sz  = max(FlagSz + 1, Ts::flags.size()...);
            static constexpr size_t descr_sz = max(DescrSz + 1, Ts::descr.size()...);
            static constexpr auto   msg      = (... + []<class T>(wrap<T>) {
                auto fmtflags = StaticStr<flag_sz>{T::flags};
                fill(fmtflags, ' ', T::flags.size() - 1, -1);
                auto fmtdescr = StaticStr<descr_sz>{T::descr};
                fill(fmtdescr, ' ', T::descr.size() - 1, -1);
                return "  -" + fmtflags + " : " + fmtdescr + " set to: {}\n";
            }(wrap_v<Ts>));
            fmt::print(msg, args.value...);
        });
    }
};

namespace detail {
    [[nodiscard]] static constexpr std::string_view get_flag_name(auto args_it) {
        auto   str = std::string_view(*args_it);
        size_t beg = str.find_first_not_of(SPACES);  // remove heading spaces
        if (str[beg] != '-')
            exit_with_message("Error parsing cli arguments: {}", str.substr(beg));

        beg       = str.find_first_not_of('-', beg);              // remove heading '-'s
        size_t sz = str.find_last_not_of(SPACES, beg) - beg + 1;  // get first word size

        // check if equal to flag
        return str.substr(beg, sz);
    }
}  // namespace detail

/// @brief A command-line argument that can be parsed by ClParser.
/// @tparam T The type of the argument value.
/// @tparam D The default value of the argument.
/// @tparam Des The description of the argument.
/// @tparam F The first flag of the argument.
/// @tparam Fs The other flags of the argument.
template <typename T, value_wrap D, StaticStr Des, StaticStr F, StaticStr... Fs>
struct CliArg {
    using key_t      = ct<F>;
    using val_wrap_t = decltype(D);

    struct value_t {
        T                     value;
        static constexpr auto flags = ((Fs + "|") + ... + F);
        static constexpr auto descr = Des + " (" + type_name<T>::local_name + ")";

        constexpr bool try_consume(auto& args_it) {
            // check if equal to flag
            if (auto w = detail::get_flag_name(args_it); ((w != F) && ... && (w != Fs)))
                return false;

            // if yes, parse the next word as T
            ++args_it;
            auto   str = std::string_view(*args_it);
            size_t beg = str.find_first_not_of(spaces);
            size_t sz  = str.find_first_of(spaces, beg) - beg;

            from_string_view_checked<T>(str.substr(beg, sz), value);
            ++args_it;

            return true;
        }
    } value = {D.value};

    [[nodiscard]] constexpr T const& operator[](ct<F> /*t*/) const {
        return value.value;
    }

    [[nodiscard]] constexpr T& operator[](ct<F> /*t*/) {
        return value.value;
    }
};

template <value_wrap<bool> D, StaticStr Des, StaticStr F, StaticStr... Fs>
struct CliArg<void, D, Des, F, Fs...> {
    using key_t = ct<F>;

    struct value_t {
        bool                  value;
        static constexpr auto flags = ((Fs + "|") + ... + F);
        static constexpr auto descr = Des;

        constexpr bool try_consume(auto& args_it) {
            if (auto w = detail::get_flag_name(args_it); ((w != F) && ... && (w != Fs)))
                return false;
            ++args_it;
            return value = true;
        }
    } value = {D.value};

    [[nodiscard]] constexpr bool operator[](ct<F> /*t*/) const {
        return value.value;
    }

    [[nodiscard]] constexpr bool& operator[](ct<F> /*t*/) {
        return value.value;
    }
};

#ifdef CAV_COMP_TESTS
namespace {
    struct MyTestCliParser
        : ClParser<CliArg<void, false, "Help msg", "help", "h", "h2">,
                   CliArg<std::string, "none", "Stirng", "string", "s">> {};

    CAV_BLOCK_PASS({
        // parsing is constexpr from c++23, for now just test string and void args
        char const* args[]    = {"./test", "-s", "string", "-h"};
        auto        args_span = std::span(args, std::size(args));
        auto        cli       = MyTestCliParser{};

        assert(!cli["help"_cs]);
        assert(cli["string"_cs] == "none");

        cli.parse_cli(args_span);
        assert(cli["help"_cs]);
        assert(cli["string"_cs] == "string");
    });

    CAV_BLOCK_FAIL({
        char const* args[]        = {"./test", "-s", "string", "-h", "true"};  //-h expects no value
        auto        args_span     = std::span(args, std::size(args));
        [[maybe_unused]] auto cli = MyTestCliParser{args_span};
    });

}  // namespace
#endif

}  // namespace cav

#endif /* CAV_CAVLIB_INCLUDE_CAV_MISH_CLARGS_HPP */
