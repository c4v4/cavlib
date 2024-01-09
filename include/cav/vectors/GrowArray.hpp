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

#ifndef CAV_INCLUDE_UTILS_GROWARRAY_HPP
#define CAV_INCLUDE_UTILS_GROWARRAY_HPP


#include <array>
#include <cassert>

#include "../comptime/syntactic_sugars.hpp"

namespace cav {


/// @brief std::array wrapper when the max size is known but it
/// also need to be used partially with std::vector-like operations.
///
/// @tparam T value type
/// @tparam N max size
template <typename T, int N>
class GrowArray : public std::array<T, N> {
public:
    using value_type = T;

    template <typename OtherT>
    requires std::same_as<no_cvr<OtherT>, T>
    constexpr void push_back(OtherT&& val) {
        assert(sz < N);
        (*this)[sz++] = FWD(val);
    }

    constexpr auto emplace_back(auto&&... args) {
        assert(sz < N);
        (*this)[sz++] = T(FWD(args)...);
        return this->begin() + (sz - 1);
    }

    [[nodiscard]] constexpr T const& back() const {
        return (*this)[sz - 1];
    }

    [[nodiscard]] constexpr T& back() {
        return (*this)[sz - 1];
    }

    [[nodiscard]] constexpr auto end() const {
        return this->begin() + sz;
    }

    [[nodiscard]] constexpr auto end() {
        return this->begin() + sz;
    }

    [[nodiscard]] constexpr bool empty() const {
        return sz == 0;
    }

    [[nodiscard]] constexpr int size() const {
        return sz;
    }

    constexpr void clear() {
        sz = 0;
    }

private:
    int sz{};
};

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_GROWARRAY_HPP */
