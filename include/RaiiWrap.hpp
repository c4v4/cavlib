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

#ifndef CAV_INCLUDE_UTILS_RAII_ADAPTER_HPP
#define CAV_INCLUDE_UTILS_RAII_ADAPTER_HPP

#include "syntactic_sugars.hpp"

namespace cav {

template <typename T, typename DestrT>
struct RaiiWrap {
    T                            val;
    [[no_unique_address]] DestrT destroyer;

    RaiiWrap(auto&& c_val, auto&& c_destroyer)
        : val(FWD(c_val))
        , destroyer(FWD(c_destroyer)){};

    RaiiWrap(RaiiWrap const&)                = delete;
    RaiiWrap& operator=(RaiiWrap const&)     = delete;
    RaiiWrap(RaiiWrap&&) noexcept            = default;
    RaiiWrap& operator=(RaiiWrap&&) noexcept = default;

    ~RaiiWrap() {
        destroyer(std::move(val));
    }

    [[nodiscard]] operator T&() {
        return val;
    }
};

template <typename T, typename DestrT>
RaiiWrap(T x, DestrT destr) -> RaiiWrap<T, DestrT>;

}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_RAII_ADAPTER_HPP */
