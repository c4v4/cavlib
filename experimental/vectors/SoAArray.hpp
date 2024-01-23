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

#ifndef CAV_INCLUDE_UTILS_SOAARRAY_HPP
#define CAV_INCLUDE_UTILS_SOAARRAY_HPP

#include <memory>

#include "../../include/cav/comptime/test.hpp"
#include "../../include/cav/mish/util_functions.hpp"
#include "../tuplish/tuple.hpp"
#include "IndexProxyIter.hpp"

namespace cav {

struct soa_tag;
struct aos_tag;

/// @brief SoAArray: abstraction that provides easy access to either Structure of Arrays (SoA) or
/// Array of Structures (AoS) data types. The interface is designed to be as similar as possible
/// between the two versions.
///
/// The SoAArray was initially designed to be a full std::vector-like container with support for
/// custom allocators and resizing. However, these features have been removed for simplicity, as
/// they are not frequently needed.
///
/// It also initially included a full abstraction for L/R value references and  pointers proxies
/// (with operator& overloading), but this was removed due to the risk of producing surprising
/// behavior in some corner cases.
///
/// In its current form, the SoAArray is a simpler data structure that focuses on providing easy
/// SoA/AoS access pattern (and simplyfy the conversion between the two)
///
/// @tparam ...Ts
template <typename, typename...>
class SoAArray;

template <typename... Ts>
class SoAArray<aos_tag, Ts...> : public OwnSpan<tuple<Ts...>> {
public:
    using value_type     = tuple<Ts...>;
    using base           = OwnSpan<value_type>;
    using const_iterator = typename base::const_iterator;

    using base::base;

    constexpr SoAArray() = default;

    constexpr SoAArray(std::input_iterator auto first, std::input_iterator auto last)
        : base{std::distance(first, last), [&](auto& me) {
                   size_t i  = 0;
                   auto   it = first;
                   while (it != last) {
                       std::construct_at(&me[i], *it);
                       ++it, ++i;
                   }
               }} {
    }

    [[nodiscard]] constexpr value_type*       data()       = delete;  // pointers break SoA
    [[nodiscard]] constexpr value_type const* data() const = delete;  // pointers break SoA
};

//////////

template <typename, typename>
struct TupleProxyLRef;

template <typename SoaT, std::size_t... Is>
struct TupleProxyLRef<SoaT, std::index_sequence<Is...>> {

    using self           = TupleProxyLRef;
    using soa_array_type = SoaT;
    using tup_type       = typename soa_array_type::value_type;

    size_t          idx;
    soa_array_type* soa_array;

    struct TupleProxyRRef : TupleProxyLRef {
        using base = TupleProxyLRef;

        [[nodiscard]] constexpr decl_auto operator[](auto i) const {
            return std::move(soa_array->base.ptrs[i][idx]);
        }

        [[nodiscard]] constexpr decl_auto reduce(auto&& fn) const {
            return base::reduce([&](auto&... x) { return FWD(fn)(std::move(x)...); });
        }

        constexpr bool for_each(auto&& fn) const {
            return base::for_each([&](auto& x) { return FWD(fn)(std::move(x)); });
        }

        constexpr bool visit_idx(size_t i, auto&& fn) const {
            return base::visit_idx(i, [&](auto& x) { return FWD(fn)(std::move(x)); });
        }

        template <typename T>
        [[nodiscard]] constexpr operator T() {
            return {std::move((*this)[ct_v<Is>])...};
        }
    };


private:
    friend SoaT;

    constexpr TupleProxyLRef(size_t c_idx, SoaT* c_soa_array)
        : idx(c_idx)
        , soa_array(c_soa_array) {
    }

    constexpr TupleProxyLRef(self const& other) = default;

public:
    constexpr self const& operator=(self other) const {
        (void)(((*this)[ct_v<Is>] = other[ct_v<Is>]), ...);
        return *this;
    }

    // Explicit to enable assignment with {}-list
    constexpr self const& operator=(tup_type const& tup) const {
        (void)(((*this)[ct_v<Is>] = tup[ct_v<Is>]), ...);
        return *this;
    }

    constexpr self const& operator=(TupleProxyRRef other) const {
        (void)(((*this)[ct_v<Is>] = std::move(other[ct_v<Is>])), ...);
        return *this;
    }

    [[nodiscard]] constexpr operator TupleProxyRRef() const {
        return {*this};
    }

    [[nodiscard]] constexpr operator tup_type() const {
        return tup_type{(*this)[ct_v<Is>]...};
    }

    [[nodiscard]] constexpr decl_auto operator[](auto i) const {
        return soa_array->base.ptrs[i][idx];
    }

    [[nodiscard]] constexpr decl_auto reduce(auto&& fn) const {
        return FWD(fn)((*this)[ct_v<Is>]...);
    }

    constexpr bool for_each(auto&& fn) const {
        return ((FWD(fn)((*this)[ct_v<Is>]), ...));
    }

    constexpr bool visit_idx(size_t i, auto&& fn) const {
        return reduce([&](auto&&... x) { return ((i-- == 0 && (FWD(fn)(FWD(x)), true)) || ...); });
    }

    [[nodiscard]] static constexpr size_t size() {
        return tup_type::size();
    }

    template <typename T>
    [[nodiscard]] explicit constexpr operator T() const {
        return {(*this)[ct_v<Is>]...};
    }
};
}  // namespace cav

namespace std {
template <typename SoaT, typename ISeqT>
[[nodiscard]] constexpr auto move(::cav::TupleProxyLRef<SoaT, ISeqT> ref)
    -> decltype(ref)::TupleProxyRRef {
    return ref;
}

}  // namespace std

namespace cav {

/// @brief SoAArray specialization for Structure of arrays.
/// Possible extensions:
/// - support for resize (make it vector-like)
/// - support for custom allocators (will I ever use them tho?)
/// - full pointer-proxy abstaction (with operator& overloading)
template <typename... Ts>
class SoAArray<soa_tag, Ts...> {
public:
    using self                   = SoAArray;
    using value_type             = tuple<Ts...>;
    using reference              = TupleProxyLRef<self, std::index_sequence_for<Ts...>>;
    using const_reference        = TupleProxyLRef<self const, std::index_sequence_for<Ts...>>;
    using iterator               = IndexProxyIter<self>;
    using const_iterator         = IndexProxyIter<self const>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using allocator_type         = std::allocator<value_type>;  // lie

    static constexpr size_t ntypes = sizeof...(Ts);

    struct soa_base {
        using ptr_tuple_t             = tuple<Ts*...>;
        static constexpr auto t_sizes = std::array<size_t, ntypes>{sizeof(Ts)...};
        static constexpr auto t_align = std::array<size_t, ntypes>{alignof(Ts)...};

        [[no_unique_address]] ptr_tuple_t ptrs = {};
        size_t                            sz   = {};

        constexpr ptr_tuple_t allocate(size_t n) {
            auto ptr = ptr_tuple_t{};
            ptr.for_each([n]<class T>(T*& ptr) { ptr = std::allocator<T>{}.allocate(n); });
            return ptr;
        }

        constexpr void destroy() {
            ptrs.for_each([this]<class T>(T*& ptr) {
                for (size_t j = 0; j < sz; ++j)
                    std::destroy_at(ptr + j);
            });
        }

        constexpr void deallocate() {
            ptrs.for_each([this]<class T>(T* ptr) { std::allocator<T>{}.deallocate(ptr, sz); });
            ptrs = {};
            sz   = 0;
        }

        constexpr soa_base() = default;

        constexpr soa_base(size_t n, value_type const& tup = {})
            : ptrs{allocate(n)}
            , sz{n} {

            for_each_idx<ntypes>([&](auto i) {
                for (size_t j = 0; j < sz; ++j)
                    std::construct_at(ptrs[i] + j, tup[i]);
            });
        }

        constexpr soa_base(soa_base const& other)
            : ptrs{allocate(other.sz)}
            , sz{other.sz} {

            for_each_idx<ntypes>([&](auto i) {
                for (size_t j = 0; j < sz; ++j)
                    std::construct_at(ptrs[i] + j, other.ptrs[i][j]);
            });
        }

        constexpr soa_base(soa_base&& other) noexcept
            : ptrs{std::exchange(other.ptrs, nullptr)}
            , sz{std::exchange(other.sz, 0)} {
        }

        constexpr soa_base(std::input_iterator auto first, std::input_iterator auto last)
            : sz(std::distance(first, last)) {
            ptrs = allocate(sz);
            for (int j = 0; auto& tup : std::span(first, last)) {
                for_each_idx<ntypes>([&](auto i) { std::construct_at(ptrs[i] + j, tup[i]); });
                ++j;
            }
        }

        constexpr soa_base& operator=(soa_base const& other) {
            if (this == &other)
                return *this;

            if (sz == other.sz) {
                for_each_idx<ntypes>([&](auto i) {
                    for (size_t j = 0; j < sz; ++j)
                        ptrs[i][j] = other.ptrs[i][j];
                });
                return *this;
            }

            destroy();
            deallocate(ptrs, sz);
            sz   = other.sz;
            ptrs = allocate(sz);
            for_each_idx<ntypes>([&](auto i) {
                for (size_t j = 0; j < sz; ++j)
                    std::construct_at(ptrs[i] + j, other.ptrs[i][j]);
            });
            return *this;
        }

        constexpr soa_base& operator=(soa_base&& other) noexcept {
            if (this == &other)
                return *this;

            destroy();
            deallocate(ptrs, sz);
            ptrs = std::exchange(other.ptrs, nullptr);
            sz   = std::exchange(other.sz, 0);
        }

        constexpr ~soa_base() {
            destroy();
            deallocate();
        }
    };

    soa_base base = {};


public:
    constexpr SoAArray() = default;

    constexpr SoAArray(auto&&... args)
        : base(FWD(args)...) {
    }

    // Element access
    [[nodiscard]] constexpr reference operator[](size_t i) noexcept {
        assert(i < size());
        return {i, this};
    }

    [[nodiscard]] constexpr const_reference operator[](size_t i) const noexcept {
        assert(i < size());
        return {i, this};
    }

    [[nodiscard]] constexpr reference front() {
        return (*this)[0];
    }

    [[nodiscard]] constexpr reference front() const {
        return (*this)[0];
    }

    [[nodiscard]] constexpr reference back() {
        return (*this)[size() - 1];
    }

    [[nodiscard]] constexpr reference back() const {
        return (*this)[size() - 1];
    }

    // Iterators
    [[nodiscard]] constexpr iterator begin() {
        return {0, this};
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return {0, this};
    }

    [[nodiscard]] constexpr iterator end() {
        return {size(), this};
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return {size(), this};
    }

    // Capacity
    [[nodiscard]] constexpr bool empty() {
        return size() == 0;
    }

    [[nodiscard]] constexpr size_type size() const {
        return base.sz;
    }

    // Modifiers
    constexpr void assign_all(value_type const& tup) noexcept {
        for_each_idx<ntypes>([&](auto i) {
            for (size_t j = 0; j < base.sz; ++j)
                base[i].ptr[i], tup[i];
        });
    }
};
}  // namespace cav

#ifdef CAV_COMP_TESTS
#include <vector>
namespace cav {
namespace {

    using aos0 = SoAArray<aos_tag>;
    using soa0 = SoAArray<soa_tag>;
    using aos1 = SoAArray<aos_tag, int>;
    using soa1 = SoAArray<soa_tag, int>;
    using aos2 = SoAArray<aos_tag, int, int>;
    using soa2 = SoAArray<soa_tag, int, int>;
    using aos3 = SoAArray<aos_tag, int, double, std::vector<int>>;
    using soa3 = SoAArray<soa_tag, int, double, std::vector<int>>;

    CAV_PASS(sizeof(aos0) == 16);
    CAV_PASS(sizeof(soa0) == 8);
    CAV_PASS(sizeof(aos1) == 16);
    CAV_PASS(sizeof(soa1) == 16);
    CAV_PASS(sizeof(aos2) == 16);
    CAV_PASS(sizeof(soa2) == 24);
    CAV_PASS(sizeof(aos3) == 16);
    CAV_PASS(sizeof(soa3) == 32);
    CAV_PASS(eq<typename aos3::value_type, typename soa3::value_type>)
    CAV_PASS(eq<typename aos0::value_type, typename soa0::value_type>)
    CAV_PASS((aos3{1, tuple{1, 2.0, std::vector<int>{1, 2, 3}}}[0][1_ct] == 2.0));
    CAV_PASS((soa3{1, tuple{1, 2.0, std::vector<int>{1, 2, 3}}}[0][2_ct][1] == 2));

    struct struct_of_3 {
        int              x;
        double           y;
        std::vector<int> z;
    };

    CAV_BLOCK_PASS({
        auto a3 = aos3{10};
        for (int i = 0; decl_auto tup : a3) {
            tup = {i, i * 0.1, std::vector<int>{i, i + 1, i + 2}};
            ++i;
        }

        a3[0]          = {1, .0, std::vector<int>{1, 2, 3}};
        a3[1]          = tuple{2, 0.2, std::vector<int>{4, 5, 6}};
        a3[2]          = a3[1];
        a3[3]          = std::move(a3[2]);
        auto x         = struct_of_3(a3[3]);
        auto y         = struct_of_3(std::move(a3[3]));
        a3[4][2_ct][2] = 999;
        std::swap(a3[4][2_ct], a3[5][2_ct]);

        assert(a3[0][0_ct] == 1 && a3[0][1_ct] == 0.0 && a3[0][2_ct][0] == 1);
        assert(a3[1][0_ct] == 2 && a3[1][1_ct] == 0.2 && a3[1][2_ct][0] == 4);
        assert(a3[2][0_ct] == 2 && a3[2][1_ct] == 0.2 && a3[2][2_ct].empty());  // moved
        assert(a3[3][0_ct] == 2 && a3[3][1_ct] == 0.2 && a3[3][2_ct].empty());  // moved
        assert(a3[4][0_ct] == 4 && a3[4][1_ct] == 0.4 && a3[4][2_ct][2] == 7);
        assert(a3[5][0_ct] == 5 && a3[5][1_ct] == 0.5 && a3[5][2_ct][2] == 999);  // swapped
        assert(x.x == 2 && x.y == 0.2 && x.z[0] == 4);
        assert(y.x == 2 && y.y == 0.2 && y.z[0] == 4);

        auto s3 = soa3{a3.begin(), a3.end()};
        assert(s3.size() == a3.size());
        assert(s3[0][0_ct] == a3[0][0_ct]);
        assert(s3[1][1_ct] == a3[1][1_ct]);
        assert(s3[2][2_ct].size() == a3[2][2_ct].size());
    });

    CAV_BLOCK_PASS({
        auto s3 = soa3{10};
        for (int i = 0; decl_auto tup : s3) {
            tup = {i, i * 0.1, std::vector<int>{i, i + 1, i + 2}};
            ++i;
        }

        s3[0]          = {1, .0, std::vector<int>{1, 2, 3}};
        s3[1]          = tuple{2, 0.2, std::vector<int>{4, 5, 6}};
        s3[2]          = s3[1];
        s3[3]          = std::move(s3[2]);
        auto x         = struct_of_3(s3[3]);
        auto y         = struct_of_3(std::move(s3[3]));
        s3[4][2_ct][2] = 999;
        std::swap(s3[4][2_ct], s3[5][2_ct]);

        assert(s3[0][0_ct] == 1 && s3[0][1_ct] == 0.0 && s3[0][2_ct][0] == 1);
        assert(s3[1][0_ct] == 2 && s3[1][1_ct] == 0.2 && s3[1][2_ct][0] == 4);
        assert(s3[2][0_ct] == 2 && s3[2][1_ct] == 0.2 && s3[2][2_ct].empty());  // moved
        assert(s3[3][0_ct] == 2 && s3[3][1_ct] == 0.2 && s3[3][2_ct].empty());  // moved
        assert(s3[4][0_ct] == 4 && s3[4][1_ct] == 0.4 && s3[4][2_ct][2] == 7);
        assert(s3[5][0_ct] == 5 && s3[5][1_ct] == 0.5 && s3[5][2_ct][2] == 999);  // swapped
        assert(x.x == 2 && x.y == 0.2 && x.z[0] == 4);
        assert(y.x == 2 && y.y == 0.2 && y.z[0] == 4);

        auto vec = std::move(s3[5])[2_ct];
        assert(vec[2] == 999 && s3[5][2_ct].empty());

        auto a3 = aos3{s3.begin(), s3.end()};
        assert(s3.size() == a3.size());
        assert(s3[0][0_ct] == a3[0][0_ct]);
        assert(s3[1][1_ct] == a3[1][1_ct]);
        assert(s3[2][2_ct].size() == a3[2][2_ct].size());
    });
}  // namespace
#endif
}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_SOAARRAY_HPP */
