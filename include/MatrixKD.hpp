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

#ifndef CAV_INCLUDE_UTILS_MATRIXKD_HPP
#define CAV_INCLUDE_UTILS_MATRIXKD_HPP

#include <bits/utility.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <span>

#include "OwnSpan.hpp"
#include "syntactic_sugars.hpp"

namespace cav {

namespace detail {
    template <typename ContT>
    [[nodiscard]] constexpr auto sub_prod(ContT const& container, size_t idx_beg) {
        if (idx_beg >= container.size())
            return typename ContT::value_type{};

        auto result = container[idx_beg];
        for (size_t i = idx_beg + 1; i < container.size(); ++i)
            result *= container[i];
        return result;
    }
}  // namespace detail

template <typename, size_t, size_t>
struct SubMatrixKD;

template <typename T, size_t K = 1>
struct MatrixKD {
    using self       = MatrixKD<T, K>;
    using value_type = T;

    OwnSpan<T>            own_span;
    std::array<size_t, K> sizes;
    size_t                rem_size;

    MatrixKD() = default;

    template <std::integral... Ts>
    requires(sizeof...(Ts) == K)
    MatrixKD(T const& default_val, Ts... c_sizes)
        : own_span{(c_sizes * ...), default_val}
        , sizes{static_cast<size_t>(c_sizes)...}
        , rem_size{detail::sub_prod(sizes, 1)} {
    }

    [[nodiscard]] size_t size() const noexcept {
        return sizes[0];
    }

    [[nodiscard]] SubMatrixKD<self, K - 1, K> operator[](size_t i) {
        assert(i < size());
        return {*this, i * rem_size, detail::sub_prod(sizes, 2)};
    }

    [[nodiscard]] SubMatrixKD<self const, K - 1, K> operator[](size_t i) const {
        assert(i < size());
        return {*this, i * rem_size, detail::sub_prod(sizes, 2)};
    }

    [[nodiscard]] T* data() {
        return own_span.data();
    }

    [[nodiscard]] T const* data() const {
        return own_span.data();
    }

    [[nodiscard]] auto& data_span() {
        return own_span;
    }

    [[nodiscard]] auto const& data_span() const {
        return own_span;
    }
};

template <typename T, typename... SzTs>
MatrixKD(T, SzTs...) -> MatrixKD<cav::no_cvr<T>, sizeof...(SzTs)>;

namespace detail {
    template <typename T>
    using get_value_t = if_t<std::is_const_v<T>,
                             typename T::value_type const,
                             typename T::value_type>;
}  // namespace detail

template <typename MatT, size_t FreeK, size_t K>
struct SubMatrixKD {
    using self                    = SubMatrixKD<MatT, FreeK, K>;
    using value_type              = detail::get_value_t<MatT>;
    static constexpr size_t fix_k = K - FreeK;

    MatT&  mat;
    size_t offset;
    size_t rem_size;

    [[nodiscard]] size_t size() const noexcept {
        return mat.sizes[fix_k];
    }

    SubMatrixKD<MatT, FreeK - 1, K> operator[](size_t i) {
        assert(i < size());
        return {mat, offset + i * rem_size, detail::sub_prod(mat.sizes, fix_k + 1)};
    }

    SubMatrixKD<MatT const, FreeK - 1, K> operator[](size_t i) const {
        assert(i < size());
        return {mat, offset + i * rem_size, detail::sub_prod(mat.sizes, fix_k + 1)};
    }

    [[nodiscard]] value_type* data() {
        return mat.data();
    }

    [[nodiscard]] value_type const* data() const {
        return mat.data();
    }
};

template <typename MatT, size_t K>
struct SubMatrixKD<MatT, 1, K> : std::span<detail::get_value_t<MatT>> {
    using value_type = detail::get_value_t<MatT>;
    using base       = std::span<value_type>;

    using base::base;
    SubMatrixKD(MatT& c_mat, size_t c_offset, size_t /*rem_size*/)
        : base(c_mat.data() + c_offset, c_mat.sizes.back()){};

    decl_auto operator[](size_t i) {
        assert(i <= base::size());
        return base::operator[](i);
    }

    decl_auto operator[](size_t i) const {
        assert(i <= base::size());
        return base::operator[](i);
    }
};

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_MATRIXKD_HPP */
