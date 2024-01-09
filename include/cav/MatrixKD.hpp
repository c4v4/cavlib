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
#include <cstdint>
#include <span>
#include <type_traits>

#include "OwnSpan.hpp"
#include "comptime_test.hpp"
#include "mp_base.hpp"
#include "syntactic_sugars.hpp"
#include "util_functions.hpp"

namespace cav {

template <typename, int>
struct SubMatrixKD;

/// @brief Retrieve the subobject of dimension K at the given offset of the matrix
template <int K, typename MatT>
[[nodiscard]] static constexpr decl_auto sub_object(MatT&& mat, uint32_t offset) {
    using mat_t = std::remove_reference_t<MatT>;
    static_assert(0 <= K && K <= mat_t::dimensions);

    if constexpr (K == mat_t::dimensions)
        return mat;

    else if constexpr (K > 0)
        return SubMatrixKD<mat_t, K>{FWD(mat), offset};

    else if constexpr (K == 0)
        return mat.own_span[offset];
}

/// @brief Matrix with K dimensions.
/// Barebones implementation to keep it readable and avoid hunderd of line of iterator shenanigans.
/// Sub-matrices are represented using SubMatrixKD proxy and have the mat[i][j] syntax.
/// Strides for each dimentsion are stored in explicitly.
/// Some compile-time testing at the end of the header.
template <typename T, int K = 1>
struct MatrixKD {
    using self       = MatrixKD<T, K>;
    using value_type = T;

    static constexpr int dimensions = K;

    OwnSpan<T> own_span   = {};
    uint32_t   sizes[K]   = {};
    uint32_t   strides[K] = {};

    constexpr MatrixKD() = default;

    template <std::integral... Ts>
    requires(sizeof...(Ts) == K)
    constexpr MatrixKD(T const& default_val, Ts... c_sizes)
        : own_span{(c_sizes * ...), default_val}
        , sizes{static_cast<uint32_t>(c_sizes)...} {

        strides[K - 1] = 1;
        for (int i = K - 2; i >= 0; --i)
            strides[i] = strides[i + 1] * sizes[i + 1];
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return sizes[0];
    }

    [[nodiscard]] constexpr size_t stride() const noexcept {
        return strides[0];
    }

    [[nodiscard]] constexpr decl_auto operator[](uint32_t i) {
        return sub_object<K - 1>(*this, i * strides[0]);
    }

    [[nodiscard]] constexpr decl_auto operator[](uint32_t i) const {
        assert(i < size());
        return sub_object<K - 1>(*this, i * strides[0]);
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

template <typename MatT, int K>
struct SubMatrixKD {
    CAV_PASS(0 < K && K < MatT::dimensions);

    static constexpr int sizes_idx = MatT::dimensions - K;

    MatT&    mat;
    uint32_t offset;

    [[nodiscard]] constexpr size_t size() const noexcept {
        return mat.sizes[sizes_idx];
    }

    [[nodiscard]] constexpr size_t stride() const noexcept {
        return mat.strides[sizes_idx];
    }

    [[nodiscard]] constexpr decl_auto operator[](size_t i) {
        assert(i < size());
        return sub_object<K - 1>(mat, offset + i * mat.strides[sizes_idx]);
    }

    [[nodiscard]] constexpr decl_auto operator[](size_t i) const {
        assert(i < size());
        return sub_object<K - 1>(mat, offset + i * mat.strides[sizes_idx]);
    }

    [[nodiscard]] constexpr auto* data() {
        return mat.data();
    }

    [[nodiscard]] constexpr auto const* data() const {
        return mat.data();
    }

    [[nodiscard]] constexpr auto data_span() {
        return std::span{mat.data() + offset, size()};
    }

    [[nodiscard]] constexpr auto data_span() const {
        return std::span{mat.data() + offset, size()};
    }
};

///////////// TESTS ////////////////
#ifdef COMP_TESTS
namespace test {
    CAV_PASS(sizeof(MatrixKD<int, 0>) == 16);
    CAV_PASS(sizeof(MatrixKD<int, 1>) == 24);
    CAV_PASS(sizeof(MatrixKD<int, 2>) == 32);
    CAV_PASS(sizeof(MatrixKD<int, 3>) == 40);
    CAV_PASS(sizeof(MatrixKD<int, 4>) == 48);

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<uint32_t, 1>{0, 24};
        for (uint32_t c = 0; uint32_t & n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 24 && cmat.stride() == 1);
        assert(cmat.data_span().size() == 24);

        for (uint32_t i = 0; i < cmat.size(); ++i)
            assert(cmat[i] == i * cmat.strides[0]);
    });

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<uint32_t, 2>{0, 4, 6};
        for (uint32_t c = 0; uint32_t & n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 4 && cmat.stride() == 6);
        assert(cmat[0].size() == 6 && cmat[0].stride() == 1);
        assert(cmat.data_span().size() == 24);

        for (uint32_t i = 0; i < cmat.size(); ++i)
            for (uint32_t j = 0; j < cmat[i].size(); ++j)
                assert(cmat[i][j] == i * cmat.strides[0] + j);
    });

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<uint32_t, 3>{0, 2, 3, 4};
        for (uint32_t c = 0; uint32_t & n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 2 && cmat.stride() == 12);
        assert(cmat[0].size() == 3 && cmat[0].stride() == 4);
        assert(cmat[0][0].size() == 4 && cmat[0][0].stride() == 1);
        assert(cmat.data_span().size() == 24);

        for (uint32_t i = 0; i < cmat.size(); ++i)
            for (uint32_t j = 0; j < cmat[i].size(); ++j)
                for (uint32_t k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == i * cmat.strides[0] + j * cmat.strides[1] + k);
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<uint32_t, 3>{0, 2, 3, 4};
        uint32_t   tot_size = cmat.data_span().size();
        for (uint32_t i = 0; i < tot_size; ++i)  // Error, using total size instead of cmat.size()
            for (uint32_t j = 0; j < cmat[i].size(); ++j)
                for (uint32_t k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == 0);  // This is ok
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<uint32_t, 3>{0, 2, 3, 4};
        uint32_t   tot_size = cmat.data_span().size();
        for (uint32_t i = 0; i < tot_size; ++i)
            for (uint32_t j = 0; j < cmat[i].size(); ++j)
                for (uint32_t k = 0; k < cmat[i][j].size(); ++k)
                    // Error, it has not been initialized in this way
                    assert(cmat[i][j][k] == i * cmat.strides[0] + j * cmat.strides[1] + k);
    });
}  // namespace test
#endif

}  // namespace cav


#endif /* CAV_INCLUDE_MATRIXKD_HPP */
