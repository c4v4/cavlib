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

#ifndef CAV_INCLUDE_MATRIXKD_HPP
#define CAV_INCLUDE_MATRIXKD_HPP

#include <bits/utility.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <span>

#include "OwnSpan.hpp"
#include "comptime_test.hpp"
#include "mp_base.hpp"
#include "syntactic_sugars.hpp"

namespace cav {

template <typename, size_t>
struct SubMatrixKD;

template <typename T, size_t K = 1>
struct MatrixKD {
    using self       = MatrixKD<T, K>;
    using value_type = T;

    static constexpr size_t dimensions = K;

    OwnSpan<T> own_span         = {};
    uint32_t   sizes[K]         = {};
    uint32_t   rem_sizes[K - 1] = {};

    constexpr MatrixKD() = default;

    template <std::integral... Ts>
    requires(sizeof...(Ts) == K)
    constexpr MatrixKD(T const& default_val, Ts... c_sizes)
        : own_span{(c_sizes * ...), default_val}
        , sizes{static_cast<uint32_t>(c_sizes)...} {

        rem_sizes[K - 2] = sizes[K - 2];
        for (int i = K - 3; i >= 0; --i)
            rem_sizes[i] = rem_sizes[i + 1] * sizes[i];
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return sizes[0];
    }

    [[nodiscard]] constexpr SubMatrixKD<self, K - 1> operator[](uint32_t i) {
        assert(i < size());
        return {*this, i * rem_sizes[0], rem_sizes[1]};
    }

    [[nodiscard]] constexpr SubMatrixKD<self const, K - 1> operator[](uint32_t i) const {
        assert(i < size());
        return {*this, i * rem_sizes[0], rem_sizes[1]};
    }

    [[nodiscard]] constexpr T* data() {
        return own_span.data();
    }

    [[nodiscard]] constexpr T const* data() const {
        return own_span.data();
    }

    [[nodiscard]] constexpr auto& data_span() {
        return own_span;
    }

    [[nodiscard]] constexpr auto const& data_span() const {
        return own_span;
    }
};

template <typename T, typename... SzTs>
MatrixKD(T, SzTs...) -> MatrixKD<cav::no_cvr<T>, sizeof...(SzTs)>;

template <typename T>
struct MatrixKD<T, 0> {
    using self       = MatrixKD<T, 0>;
    using value_type = T;

    static constexpr size_t dimensions = 0;

    constexpr MatrixKD() = default;

    constexpr MatrixKD(T const& default_val) {
    }

    [[nodiscard]] static constexpr size_t size() noexcept {
        return 0;
    }

    [[nodiscard]] constexpr SubMatrixKD<self, 0> operator[](size_t /*i*/) noexcept {
        CAV_PASS(always_false<self>, "Cannot index a 0-dimensional matrix");
        return {};
    }

    [[nodiscard]] static constexpr T* data() noexcept {
        return nullptr;
    }

    [[nodiscard]] static constexpr auto data_span() noexcept {
        return OwnSpan<T>{};
    }
};

namespace detail {

    template <typename T>
    using get_value_t = if_t<std::is_const_v<T>,
                             typename T::value_type const,
                             typename T::value_type>;
}  // namespace detail

template <typename MatT, size_t FreeK>
struct SubMatrixKD {
    CAV_PASS(0 <= FreeK && FreeK <= MatT::dimensions);

    using self                    = SubMatrixKD<MatT, FreeK>;
    using value_type              = detail::get_value_t<MatT>;
    static constexpr size_t fix_k = MatT::dimensions - FreeK;

    MatT&    mat;
    uint32_t offset;
    uint32_t rem_size;

    [[nodiscard]] constexpr size_t size() const noexcept {
        return mat.sizes[fix_k];
    }

    constexpr SubMatrixKD<MatT, FreeK - 1> operator[](size_t i) {
        assert(i < size());
        return {mat, offset + i * rem_size, mat.rem_sizes[fix_k]};
    }

    constexpr SubMatrixKD<MatT const, FreeK - 1> operator[](size_t i) const {
        assert(i < size());
        return {mat, offset + i * rem_size, mat.rem_sizes[fix_k]};
    }

    [[nodiscard]] constexpr value_type* data() {
        return mat.data();
    }

    [[nodiscard]] constexpr value_type const* data() const {
        return mat.data();
    }
};

template <typename MatT>
struct SubMatrixKD<MatT, 1> : std::span<detail::get_value_t<MatT>> {
    using value_type = detail::get_value_t<MatT>;
    using base       = std::span<value_type>;

    using base::base;

    constexpr SubMatrixKD(MatT& c_mat, size_t c_offset, size_t /*rem_size*/)
        : base(c_mat.data() + c_offset, c_mat.sizes[MatT::dimensions - 1]){};

    constexpr decl_auto operator[](size_t i) {
        assert(i <= base::size());
        return base::operator[](i);
    }

    constexpr decl_auto operator[](size_t i) const {
        assert(i <= base::size());
        return base::operator[](i);
    }
};

///////////// TESTS ////////////////
#ifdef COMP_TESTS
namespace test {
    CAV_PASS(std::is_empty_v<MatrixKD<int, 0>>);
    CAV_PASS(sizeof(MatrixKD<int, 1>) == 24);
    CAV_PASS(sizeof(MatrixKD<int, 2>) == 32);
    CAV_PASS(sizeof(MatrixKD<int, 3>) == 40);
    CAV_PASS(sizeof(MatrixKD<int, 4>) == 48);

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<int, 3>{0, 2, 2, 4};
        for (int c = 0; int& n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 2);
        assert(cmat[0].size() == 2);
        assert(cmat[0][0].size() == 4);
        assert(cmat.data_span().size() == 16);

        for (int i = 0; i < cmat.size(); ++i)
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == i * cmat.rem_sizes[0] + j * cmat.rem_sizes[1] + k);

        return true;
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<int, 3>{0, 2, 2, 4};
        size_t     tot_size = cmat.data_span().size();
        for (int i = 0; i < tot_size; ++i)  // Error, using total size instead of cmat.size()
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == 0);  // This is ok
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<int, 3>{0, 2, 2, 4};
        size_t     tot_size = cmat.data_span().size();
        for (int i = 0; i < cmat.size(); ++i)
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    // Error, it has not been initialized in this way
                    assert(cmat[i][j][k] == i * cmat.rem_sizes[0] + j * cmat.rem_sizes[1] + k);
    });

    using mat0_t              = MatrixKD<int, 0>;
    static constexpr auto mat = mat0_t{};
    CAV_PASS(mat0_t::size() == 0);
    CAV_PASS(mat0_t::data_span().empty());
    CAV_PASS(mat0_t::data() == nullptr);
}  // namespace test
#endif

}  // namespace cav


#endif /* CAV_INCLUDE_MATRIXKD_HPP */
