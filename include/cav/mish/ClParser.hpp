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

template <typename... ArTs>
struct ClParser : cav::type_map<ArTs...> {
    using base = cav::type_map<ArTs...>;

    constexpr ClParser() = default;

    template <typename T>
    ClParser(std::span<T> const& args) {
        parse_cli(args);
    }

    template <typename T>
    void parse_cli(std::span<T> const& args) {
        for (auto it = args.begin() + 1; it != args.end();)
            if (!base::for_each([&](auto& arg) { return arg.try_consume(it); }))
                cav::exit_with_message("Error: unknown argument: {}\n", *it);
    }

    template <size_t FlagSz = 10, size_t DescrSz = 36>
    void print_args() const {
        this->reduce([]<class... Ts>(Ts const&... args) {  // comptime formatting
            static constexpr auto msg = (... + []<class T>(cav::wrap<T>) {
                auto fmtflags = cav::StaticStr<cav::max(T::flags.size(), FlagSz + 1)>{T::flags};
                cav::fill(fmtflags, ' ', T::flags.size() - 1, -1);
                auto fmtdescr = cav::StaticStr<cav::max(T::descr.size(), DescrSz + 1)>{T::descr};
                cav::fill(fmtdescr, ' ', T::descr.size() - 1, -1);
                return "  -" + fmtflags + ": " + fmtdescr + " set to: {}\n";
            }(cav::wrap_v<Ts>));
            fmt::print(msg, args.value...);
        });
    }
};

namespace detail {
    [[nodiscard]] static constexpr std::string_view get_flag_name(auto args_it) {
        auto   str = std::string_view(*args_it);
        size_t beg = str.find_first_not_of(SPACES);  // remove heading spaces
        if (str[beg] != '-')
            cav::exit_with_message("Error parsing cli arguments: {}", str.substr(beg));

        beg       = str.find_first_not_of('-', beg);       // remove heading '-'s
        size_t sz = str.find_first_of(SPACES, beg) - beg;  // get first word size

        // check if equal to flag
        return str.substr(beg, sz);
    }
}  // namespace detail

template <typename T,
          cav::value_wrap Dflt,
          cav::StaticStr  Descr,
          cav::StaticStr  Flg,
          cav::StaticStr... Flgs>
struct CliArg {
    using key_t = cav::ct<Flg>;

    struct value_t {
        T value;

        static constexpr auto flags = ((Flgs + "|") + ... + Flg);
        static constexpr auto descr = Descr + " (" + cav::type_name<T>::local_name + ")";

        constexpr bool try_consume(auto& args_it) {
            // check if equal to flag
            if (auto w = detail::get_flag_name(args_it); ((w != Flg) && ... && (w != Flgs)))
                return false;

            // if yes, parse the next word as T
            ++args_it;
            auto   str = std::string_view(*args_it);
            size_t beg = str.find_first_not_of(spaces);
            size_t sz  = str.find_first_of(spaces, beg) - beg;

            cav::from_string_view_checked<T>(str.substr(beg, sz), value);
            ++args_it;

            return true;
        }
    } value = {Dflt.value};

    [[nodiscard]] constexpr T const& operator[](cav::ct<Flg> /*t*/) const {
        return value.value;
    }

    [[nodiscard]] constexpr T& operator[](cav::ct<Flg> /*t*/) {
        return value.value;
    }
};

template <cav::value_wrap D, cav::StaticStr Descr, cav::StaticStr F, cav::StaticStr... Fs>
struct CliArg<void, D, Descr, F, Fs...> {
    using key_t = cav::ct<F>;

    struct value_t {
        bool                  value;
        static constexpr auto flags = ((Fs + "|") + ... + F);
        static constexpr auto descr = Descr;

        constexpr bool try_consume(auto& args_it) {
            auto w = detail::get_flag_name(args_it);
            value  = ((w == F) || ... || (w == Fs));
            if (value)
                ++args_it;
            return value;
        }
    } value;

    [[nodiscard]] constexpr bool operator[](cav::ct<F> /*t*/) const {
        return value.value;
    }

    [[nodiscard]] constexpr bool& operator[](cav::ct<F> /*t*/) {
        return value.value;
    }
};

}  // namespace cav

#endif /* CAV_CAVLIB_INCLUDE_CAV_MISH_CLARGS_HPP */
