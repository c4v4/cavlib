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

#ifndef CAV_INCLUDE_RAII_ADAPTER_HPP
#define CAV_INCLUDE_RAII_ADAPTER_HPP

#include <cassert>

#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"

namespace cav {

template <typename T, typename DestrT>
struct RaiiWrap {
    T                            val;
    [[no_unique_address]] DestrT destroyer;

    constexpr RaiiWrap(auto&& c_val, auto&& c_destroyer)
        : val(FWD(c_val))
        , destroyer(FWD(c_destroyer)){};

    constexpr RaiiWrap(RaiiWrap const&)                = delete;
    constexpr RaiiWrap& operator=(RaiiWrap const&)     = delete;
    constexpr RaiiWrap(RaiiWrap&&) noexcept            = default;
    constexpr RaiiWrap& operator=(RaiiWrap&&) noexcept = default;

    constexpr ~RaiiWrap() {
        destroyer(std::move(val));
    }

    [[nodiscard]] constexpr operator T&() {
        return val;
    }
};

template <typename T, typename DestrT>
RaiiWrap(T x, DestrT destr) -> RaiiWrap<T, DestrT>;

#ifdef CAV_COMP_TESTS
namespace {
    CAV_BLOCK_PASS({
        auto ptr = RaiiWrap{new int{42}, [](auto p) { delete p; }};
        assert(*ptr == 42);
    });

    CAV_BLOCK_PASS({
        auto ptr = RaiiWrap{new int{42}, [](auto /*p*/) {}};  // useless but ok
        assert(*ptr == 42);
        delete ptr.val;
    });


    CAV_BLOCK_FAIL({
        auto ptr = RaiiWrap{new int{42}, [](auto /*p*/) {}};  // No delete -> UB
        assert(*ptr == 42);
    });

    CAV_BLOCK_FAIL({
        auto ptr = RaiiWrap{new int{42}, [](auto /*p*/) { delete p; }};
        delete ptr.val;  // double free -> UB
    });

    CAV_BLOCK_FAIL({
        auto ptr = RaiiWrap{new int{42}, [](auto /*p*/) {}};
        delete ptr.val;
        assert(*ptr == 42);  // use after free -> UB
    });

}  // namespace
#endif

}  // namespace cav

#endif /* CAV_INCLUDE_RAII_ADAPTER_HPP */
