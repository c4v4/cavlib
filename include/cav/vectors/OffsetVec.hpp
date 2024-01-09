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

#ifndef CAV_INCLUDE_UTILS_OFFSETVEC_HPP
#define CAV_INCLUDE_UTILS_OFFSETVEC_HPP

#include <cstddef>
#include <vector>

#include "../comptime/macros.hpp"
#include "../comptime/mp_utils.hpp"

namespace cav {

template <typename ContT>
class OffSetVec {
    using value_type     = typename ContT::value_type;
    using container_type = ContT;
    using iterator       = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    container_type data;
    ptrdiff_t      beg_offset = {};
    ptrdiff_t      offset     = {};

public:
    template <typename... vArgs>
    OffSetVec(size_t c_offset, vArgs&&... vargs)
        : data(FWD(vargs)...)
        , offset(beg_offset + c_offset) {
        assert(offset <= ssize(data));
    }

    [[nodiscard]] ptrdiff_t get_offset() const {
        return offset - beg_offset;
    }

    decl_auto operator[](ptrdiff_t n) noexcept {
        assert(-offset <= n && n < ssize(data) - offset);
        return data[offset + n];
    }

    decl_auto operator[](ptrdiff_t n) const noexcept {
        assert(-offset <= n && n < ssize(data) - offset);
        return data[offset + n];
    }

    decl_auto front() noexcept {
        return data[beg_offset];
    }

    decl_auto front() const noexcept {
        return data[beg_offset];
    }

    decl_auto back() noexcept {
        return data.back();
    }

    decl_auto back() const noexcept {
        return data.back();
    }

    auto begin() noexcept {
        return data.begin() + beg_offset;
    }

    auto begin() const noexcept {
        return data.begin() + beg_offset;
    }

    auto mid() noexcept {
        return data.begin() + offset;
    }

    auto mid() const noexcept {
        return data.begin() + offset;
    }

    auto end() noexcept {
        return data.end();
    }

    auto end() const noexcept {
        return data.end();
    }

    auto size() const noexcept {
        return data.size() - beg_offset;
    }

    [[nodiscard]] int beg_idx() const noexcept {
        return -offset;
    }

    [[nodiscard]] int end_idx() const noexcept {
        return data.size() - offset;
    }

    decl_auto emplace_back(auto&&... args) {
        return data.emplace_back(FWD(args)...);
    }

    decl_auto emplace_front(auto&&... args) {
        if (beg_offset == 0) {
            beg_offset = ssize(data) / 2;
            offset += beg_offset;
            data.insert(data.begin(), beg_offset, value_type{});
        }
        --beg_offset;
        data[beg_offset] = value_type{FWD(args)...};
        return data[beg_offset];
    }

    void pop_back() {
        data.pop_back();
    }

    void pop_front() {
        ++beg_offset;
    }
};
}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_OFFSETVEC_HPP */
