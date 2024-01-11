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
#include <numeric>
#include <span>
#include <type_traits>

#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"
#include "../mish/util_functions.hpp"
#include "../vectors/OwnSpan.hpp"

namespace cav {


template <typename, int>
struct SubMatrixKD;

/// @brief Retrieve the subobject of dimension K at the given offset of the matrix
template <int K, typename MatT>
[[nodiscard]] static constexpr decl_auto sub_object(MatT&& mat, int offset) {
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

    OwnSpan<T>         own_span = {};
    std::array<int, K> sizes    = {};
    std::array<int, K> strides  = {};

    constexpr MatrixKD() = default;

    template <std::integral... Ts>
    requires(sizeof...(Ts) == K)
    constexpr MatrixKD(T const& default_val, Ts... c_sizes)
        : own_span{(c_sizes * ...), default_val}
        , sizes{static_cast<int>(c_sizes)...}
        , strides{static_cast<int>(c_sizes)...} {

        auto rbeg = strides.rbegin(), rend = strides.rend();
        std::exclusive_scan(rbeg, rend, rbeg, 1, [](int x, int y) { return x * y; });
    }

    [[nodiscard]] constexpr int size() const noexcept {
        return sizes[0];
    }

    [[nodiscard]] constexpr int stride() const noexcept {
        return strides[0];
    }

    [[nodiscard]] constexpr decl_auto operator[](int i) {
        assert(0 <= i && i < size());
        return sub_object<K - 1>(*this, i * stride());
    }

    [[nodiscard]] constexpr decl_auto operator[](int i) const {
        assert(0 <= i && i < size());
        return sub_object<K - 1>(*this, i * stride());
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
    static constexpr int sizes_idx = MatT::dimensions - K;
    CAV_PASS(0 < K && K < MatT::dimensions);
    CAV_PASS(0 < sizes_idx && sizes_idx < MatT::dimensions);

    MatT& mat;
    int   offset;

    [[nodiscard]] constexpr int size() const noexcept {
        return mat.sizes[sizes_idx];
    }

    [[nodiscard]] constexpr int stride() const noexcept {
        if constexpr (K == 1)  // small optimization
            return 1;
        else
            return mat.strides[sizes_idx];
    }

    [[nodiscard]] constexpr decl_auto operator[](int i) {
        assert(0 <= i && i < size());
        return sub_object<K - 1>(mat, offset + i * stride());
    }

    [[nodiscard]] constexpr decl_auto operator[](int i) const {
        assert(0 <= i && i < size());
        return sub_object<K - 1>(mat, offset + i * stride());
    }

    [[nodiscard]] constexpr auto* data() {
        return mat.data();
    }

    [[nodiscard]] constexpr auto const* data() const {
        return mat.data();
    }

    [[nodiscard]] constexpr auto data_span() {
        return std::span{mat.data() + offset, static_cast<size_t>(size())};
    }

    [[nodiscard]] constexpr auto data_span() const {
        return std::span{mat.data() + offset, static_cast<size_t>(size())};
    }
};

///////////// TESTS ////////////////
#ifdef COMP_TESTS
namespace test {
    CAV_PASS(sizeof(MatrixKD<int, 1>) == 24);
    CAV_PASS(sizeof(MatrixKD<int, 2>) == 32);
    CAV_PASS(sizeof(MatrixKD<int, 3>) == 40);
    CAV_PASS(sizeof(MatrixKD<int, 4>) == 48);

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<int, 1>{0, 24};
        for (int c = 0; int& n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 24 && cmat.stride() == 1);
        assert(cmat.data_span().size() == 24);

        for (int i = 0; i < cmat.size(); ++i)
            assert(cmat[i] == i * cmat.strides[0]);
    });

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<int, 2>{0, 4, 6};
        for (int c = 0; int& n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 4 && cmat.stride() == 6);
        assert(cmat[0].size() == 6 && cmat[0].stride() == 1);
        assert(cmat.data_span().size() == 24);

        for (int i = 0; i < cmat.size(); ++i)
            for (int j = 0; j < cmat[i].size(); ++j)
                assert(cmat[i][j] == i * cmat.stride() + j);
    });

    CAV_BLOCK_PASS({
        auto mat = MatrixKD<int, 3>{0, 2, 3, 4};
        for (int c = 0; int& n : mat.data_span())
            n = c++;

        auto const& cmat = mat;
        assert(cmat.size() == 2 && cmat.stride() == 12);
        assert(cmat[0].size() == 3 && cmat[0].stride() == 4);
        assert(cmat[0][0].size() == 4 /* && cmat[0][0].stride() == 1 */);
        assert(cmat.data_span().size() == 24);

        for (int i = 0; i < cmat.size(); ++i)
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == i * cmat.stride() + j * cmat[i].stride() + k);
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<int, 3>{0, 2, 3, 4};
        int        tot_size = cmat.data_span().size();
        for (int i = 0; i < tot_size; ++i)  // Error, using total size instead of cmat.size()
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    assert(cmat[i][j][k] == 0);  // This is ok
    });

    CAV_BLOCK_FAIL({
        auto const cmat     = MatrixKD<int, 3>{0, 2, 3, 4};
        int        tot_size = cmat.data_span().size();
        for (int i = 0; i < tot_size; ++i)
            for (int j = 0; j < cmat[i].size(); ++j)
                for (int k = 0; k < cmat[i][j].size(); ++k)
                    // Error, it has not been initialized in this way
                    assert(cmat[i][j][k] == i * cmat.stride() + j * cmat[i].stride() + k);
    });
}  // namespace test
#endif

}  // namespace cav


#endif /* CAV_INCLUDE_MATRIXKD_HPP */
