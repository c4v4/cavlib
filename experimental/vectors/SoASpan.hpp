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

#ifndef CAV_INCLUDE_UTILS_SOASPAN_HPP
#define CAV_INCLUDE_UTILS_SOASPAN_HPP

#include <bits/utility.h>

#include <vector>

#include "../../include/cav/comptime/instance_of.hpp"
#include "../../include/cav/mish/util_functions.hpp"
#include "../tuplish/tuple.hpp"

namespace cav {

struct soa_tag;
struct aos_tag;

template <typename, typename...>
class SoASpan;

/// @brief SoASpan specialization for Array of structures. This is the simple
/// case, just inheriting from a OwnSpan of tuples.
/// @tparam ...Ts
template <typename... Ts>
class SoASpan<aos_tag, Ts...> : public OwnSpan<tuple<Ts...>> {
public:
    using value_type     = tuple<Ts...>;
    using base           = OwnSpan<value_type>;
    using const_iterator = typename base::const_iterator;

    using base::base;

    constexpr SoASpan() = default;

    constexpr SoASpan(std::size_t sz, auto&&... args)
    requires(sizeof...(args) == sizeof...(Ts))
        : base(sz, value_type{implicit_cast<Ts>(FWD(args))...}) {
    }

    constexpr void assign(std::size_t n, auto&&... args)
    requires(sizeof...(args) == sizeof...(Ts))
    {
        base::assign(n, value_type{implicit_cast<Ts>(FWD(args))...});
    }

    constexpr decl_auto emplace(const_iterator pos, auto&&... args) {
        return base::emplace(pos, value_type{implicit_cast<Ts>(FWD(args))...});
    }

    constexpr decl_auto emplace_back(auto&&... args) {
        return base::emplace_back(value_type{implicit_cast<Ts>(FWD(args))...});
    }
};

//////////

template <typename SV, typename = void>
struct TupleProxy;

template <typename... Ts, std::size_t... Is>
struct TupleProxy<SoASpan<soa_tag, Ts...>, std::index_sequence<Is...>> {

    using self       = TupleProxy;
    using soa_span_t = SoASpan<soa_tag, Ts...>;
    using tup_type   = typename soa_span_t::value_type;

    size_t      idx;
    soa_span_t* soa_span;

    struct MovingTuple : TupleProxy {
        template <typename T>
        [[nodiscard]] constexpr operator T() {
            return {std::move((*this)[ct_v<Is>])...};
        }
    };

    constexpr self& operator=(self other) noexcept {
        (void)(((*this)[ct_v<Is>] = other[ct_v<Is>]), ...);
        return *this;
    }

    // Explicit to enable assignment with {}-list
    constexpr self& operator=(tup_type const& tup) {
        (void)(((*this)[ct_v<Is>] = tup[ct_v<Is>]), ...);
        return *this;
    }

    constexpr self& operator=(MovingTuple other) {
        (void)(((*this)[ct_v<Is>] = std::move(other[ct_v<Is>])), ...);
        return *this;
    }

    [[nodiscard]] constexpr MovingTuple get_move_ref() const {
        return {idx, soa_span};
    }

    [[nodiscard]] constexpr operator tup_type() const {
        return tup_type{(*this)[ct_v<Is>]...};
    }

    [[nodiscard]] constexpr decl_auto operator[](auto i) const {
        return soa_span->base[i][idx];
    }

    [[nodiscard]] constexpr decl_auto reduce(auto&& fn) const {
        return FWD(fn)((*this)[ct_v<Is>]...);
    }

    constexpr void for_each(auto&& fn) const {
        (void)((FWD(fn)((*this)[ct_v<Is>]), ...));
    }

    constexpr void visit_idx(size_t idx, auto&& fn) const {
        assert(idx < size());
        size_t count = 0;
        (void)((count++ == idx && (FWD(fn)((*this)[ct_v<Is>]), true)) || ...);
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
template <typename SoAT, typename ISeqT>
[[nodiscard]] constexpr auto move(::cav::TupleProxy<SoAT, ISeqT>& ref) {
    return ref.get_move_ref();
}

template <typename SoAT, typename ISeqT>
[[nodiscard]] constexpr auto move(::cav::TupleProxy<SoAT, ISeqT>&& ref) {
    return ref.get_move_ref();
}

}  // namespace std

namespace cav {

template <typename>
struct SoAIterator;

/// @brief SoASpan specialization for Structure of arrays.
///        TODO(cava): add support for resize (make it vector-like)
///        TODO(cava): add support for custom allocators (will I ever use them tho?)
template <typename... Ts>
class SoASpan<soa_tag, Ts...> {
public:
    using self                   = SoASpan;
    using value_type             = tuple<Ts...>;
    using pointer                = tuple<Ts*...>;
    using const_pointer          = tuple<Ts const*...>;
    using reference              = TupleProxy<self, std::index_sequence_for<Ts...>>;
    using const_reference        = TupleProxy<self const, std::index_sequence_for<Ts...>>;
    using iterator               = SoAIterator<self>;
    using const_iterator         = SoAIterator<self const>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using allocator_type         = std::allocator<value_type>;  // lie

    static constexpr size_t ntypes = sizeof...(Ts);

    struct soa_base {

        static constexpr auto t_sizes = std::array<size_t, ntypes>{sizeof(Ts)...};
        static constexpr auto t_align = std::array<size_t, ntypes>{alignof(Ts)...};

        [[no_unique_address]] pointer ptrs = {};
        size_t                        sz   = {};

        constexpr pointer allocate(size_t n) {
            auto ptr = pointer{};
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

        constexpr soa_base(size_t n)
            : ptrs{allocate(n)}
            , sz{n} {

            for_each_idx<ntypes>([&](auto i) {
                for (size_t j = 0; j < sz; ++j)
                    std::construct_at(ptrs[i] + j);
            });
        }

        constexpr soa_base(size_t n, value_type const& tup)
            : ptrs{allocate(n)}
            , sz{n} {

            for_each_idx<ntypes>([&](auto i) {
                for (size_t j = 0; j < sz; ++j)
                    std::construct_at(ptrs[i] + j, tup[i]);
            });
        }

        constexpr soa_base(size_t n, auto&&... args)
            : ptrs{allocate(n)}
            , sz{n} {

            for_each_elem(
                [&](auto i, auto&& x) {
                    for (size_t j = 0; j < sz; ++j)
                        std::construct_at(ptrs[i] + j, FWD(x));
                },
                FWD(args)...);
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

        [[nodiscard]] constexpr auto operator[](auto i) {
            return std::span(ptrs[i], ptrs[i] + sz);
        }

        [[nodiscard]] constexpr auto operator[](auto i) const {
            return std::span(ptrs[i], ptrs[i] + sz);
        }
    };

    soa_base base = {};


public:
    constexpr SoASpan() = default;

    constexpr SoASpan(size_type sz)
        : base{sz} {
    }

    constexpr SoASpan(size_type sz, auto&&... args)
    requires(sizeof...(args) == ntypes)
        : base(sz, FWD(args)...) {
    }

    constexpr SoASpan(size_type sz, auto const& tup)
        : base{sz, tup} {
    }

    template <typename InputIter>
    constexpr SoASpan(InputIter first, InputIter last) {
        insert(begin(), first, last);
    }

    // Element access
    [[nodiscard]] constexpr auto at(size_type idx) {
        if (idx >= size())
            throw std::out_of_range{"SoASpan::at"};
        return (*this)[idx];
    }

    [[nodiscard]] constexpr auto at(size_type idx) const {
        if (idx >= size())
            throw std::out_of_range{"SoASpan::at"};
        return (*this)[idx];
    }

    [[nodiscard]] constexpr reference operator[](size_t idx) noexcept {
        assert(idx < size());
        return {idx, this};
    }

    [[nodiscard]] constexpr const_reference operator[](size_t idx) const noexcept {
        assert(idx < size());
        return {idx, this};
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

    [[nodiscard]] constexpr pointer data() {
        return base.reduce([&](auto&... vs) { return pointer{vs.data()...}; });
    }

    [[nodiscard]] constexpr const_pointer data() const {
        return base.reduce([&](auto const&... vs) { return const_pointer{vs.data()...}; });
    }

    // Iterators
    [[nodiscard]] constexpr auto begin() {
        return iterator{0, this};
    }

    [[nodiscard]] constexpr auto begin() const {
        return const_iterator{0, this};
    }

    [[nodiscard]] constexpr auto cbegin() const {
        return const_iterator{0, this};
    }

    [[nodiscard]] constexpr auto end() {
        return iterator{size(), this};
    }

    [[nodiscard]] constexpr auto end() const {
        return const_iterator{size(), this};
    }

    [[nodiscard]] constexpr auto cend() const {
        return const_iterator{size(), this};
    }

    [[nodiscard]] constexpr auto rbegin() {
        return reverse_iterator{0, this};
    }

    [[nodiscard]] constexpr auto crbegin() const {
        return const_reverse_iterator{0, this};
    }

    [[nodiscard]] constexpr auto rend() {
        return reverse_iterator{size(), this};
    }

    [[nodiscard]] constexpr auto crend() const {
        return const_reverse_iterator{size(), this};
    }

    // Capacity
    [[nodiscard]] constexpr auto empty() {
        return size() == 0;
    }

    [[nodiscard]] constexpr auto size() const {
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

template <inst_of<SoASpan> SV>
struct SoAIterator<SV> {
    using self              = SoAIterator;
    using iterator_category = typename std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;
    using value_type        = typename SV::reference;
    using pointer           = value_type*;
    using reference         = value_type&;

    std::size_t idx;
    SV*         soa_vec;

    [[nodiscard]] constexpr operator SoAIterator<const SV>() const {
        return {idx, soa_vec};
    }

    [[nodiscard]] constexpr decl_auto operator*() const {
        return soa_vec->operator[](idx);
    }

    [[nodiscard]] constexpr decl_auto operator[](ptrdiff_t n) const {
        return (*soa_vec)[idx + n];
    }

    constexpr self& operator++() {
        ++idx;
        return *this;
    }

    constexpr self operator++(int) {
        ++idx;
        return {idx - 1, soa_vec};
    }

    constexpr self& operator--() {
        --idx;
        return *this;
    }

    constexpr self operator--(int) {
        --idx;
        return {idx + 1, soa_vec};
    }

    constexpr self& operator+=(ptrdiff_t n) {
        idx += n;
        return *this;
    }

    constexpr self& operator-=(ptrdiff_t n) {
        idx -= n;
        return *this;
    }

    [[nodiscard]] constexpr ptrdiff_t operator-(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx - other.idx;
    }

    [[nodiscard]] constexpr bool operator==(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx == other.idx;
    }

    [[nodiscard]] constexpr auto operator<=>(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx <=> other.idx;
    }
};

template <inst_of<SoASpan> SV>
SoAIterator<SV> operator+(SoAIterator<SV> iter, ptrdiff_t n) {
    return iter += n;
}

template <inst_of<SoASpan> SV>
SoAIterator<SV> operator+(ptrdiff_t n, SoAIterator<SV> iter) {
    return iter += n;
}

template <inst_of<SoASpan> SV>
SoAIterator<SV> operator-(SoAIterator<SV> iter, ptrdiff_t n) {
    return iter -= n;
}

template <inst_of<SoASpan> SV>
SoAIterator<SV> operator-(ptrdiff_t n, SoAIterator<SV> iter) {
    return iter -= n;
}

static_assert(std::random_access_iterator<SoAIterator<SoASpan<soa_tag, int>>>);

template <typename SV1, typename SV2>
void swap(cav::TupleProxy<SV1> r1, cav::TupleProxy<SV2> r2) {
    using value_type = typename SV1::value_type;
    static_assert(std::same_as<value_type, typename SV2::value_type>);
    for_each_idx<value_type::size()>([&](auto i) { std::swap(r1[i], r2[i]); });
}


#ifdef CAV_COMP_TESTS
namespace {

    using aos0 = SoASpan<aos_tag>;
    using aos1 = SoASpan<aos_tag, int>;
    using aos2 = SoASpan<aos_tag, int, int>;
    using aos3 = SoASpan<aos_tag, int, double, std::vector<int>>;

    CAV_PASS(sizeof(aos0) == 16);
    CAV_PASS(sizeof(aos1) == 16);
    CAV_PASS(sizeof(aos2) == 16);
    CAV_PASS(sizeof(aos3) == 16);

    using soa0 = SoASpan<soa_tag>;
    using soa1 = SoASpan<soa_tag, int>;
    using soa2 = SoASpan<soa_tag, int, int>;
    using soa3 = SoASpan<soa_tag, int, double, std::vector<int>>;

    CAV_PASS(sizeof(soa0) == 8);
    CAV_PASS(sizeof(soa1) == 16);
    CAV_PASS(sizeof(soa2) == 24);
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

        a3[0]  = {1, .0, std::vector<int>{1, 2, 3}};
        a3[1]  = tuple{2, 0.2, std::vector<int>{4, 5, 6}};
        a3[2]  = a3[1];
        a3[3]  = std::move(a3[2]);
        auto x = struct_of_3(a3[3]);
        auto y = struct_of_3(std::move(a3[3]));

        assert(a3[0][0_ct] == 1 && a3[0][1_ct] == 0.0 && a3[0][2_ct][0] == 1);
        assert(a3[1][0_ct] == 2 && a3[1][1_ct] == 0.2 && a3[1][2_ct][0] == 4);
        assert(a3[2][0_ct] == 2 && a3[2][1_ct] == 0.2 && a3[2][2_ct].empty());  // moved
        assert(a3[3][0_ct] == 2 && a3[3][1_ct] == 0.2 && a3[3][2_ct].empty());  // moved
        assert(a3[4][0_ct] == 4 && a3[4][1_ct] == 0.4 && a3[4][2_ct][2] == 6);
        assert(x.x == 2 && x.y == 0.2 && x.z[0] == 4);
        assert(y.x == 2 && y.y == 0.2 && y.z[0] == 4);
    });

    CAV_BLOCK_PASS({
        auto s3 = soa3{10};
        for (int i = 0; decl_auto tup : s3) {
            tup = {i, i * 0.1, std::vector<int>{i, i + 1, i + 2}};
            ++i;
        }

        s3[0]  = {1, .0, std::vector<int>{1, 2, 3}};
        s3[1]  = tuple{2, 0.2, std::vector<int>{4, 5, 6}};
        s3[2]  = s3[1];
        s3[3]  = std::move(s3[2]);
        auto x = struct_of_3(s3[3]);
        auto y = struct_of_3(std::move(s3[3]));

        assert(s3[0][0_ct] == 1 && s3[0][1_ct] == 0.0 && s3[0][2_ct][0] == 1);
        assert(s3[1][0_ct] == 2 && s3[1][1_ct] == 0.2 && s3[1][2_ct][0] == 4);
        assert(s3[2][0_ct] == 2 && s3[2][1_ct] == 0.2 && s3[2][2_ct].empty());  // moved
        assert(s3[3][0_ct] == 2 && s3[3][1_ct] == 0.2 && s3[3][2_ct].empty());  // moved
        assert(s3[4][0_ct] == 4 && s3[4][1_ct] == 0.4 && s3[4][2_ct][2] == 6);
        assert(x.x == 2 && x.y == 0.2 && x.z[0] == 4);
        assert(y.x == 2 && y.y == 0.2 && y.z[0] == 4);
    });
}  // namespace
#endif
}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_SOASPAN_HPP */
