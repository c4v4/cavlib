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

#ifdef CAV_INCLUDE_UTILS_SOAVEC_HPP
#define CAV_INCLUDE_UTILS_SOAVEC_HPP

#include <vector>

#include "../Utils/tuple.hpp"
#include "../comptime/mp_utils.hpp"

namespace cav {

struct soa_tag;
struct aos_tag;

template <typename, typename...>
class SoAVec;

/// @brief SoAVec specialization for Array of structures. This is the simple
/// case, just inheriting from a std::vector of tuples.
/// @tparam ...Ts
template <typename... Ts>
class SoAVec<aos_tag, Ts...> : public std::vector<tuple<Ts...>> {
public:
    using value_type     = tuple<Ts...>;
    using base           = std::vector<value_type>;
    using const_iterator = typename base::const_iterator;

    using base::base;

    constexpr SoAVec() = default;

    constexpr SoAVec(std::size_t sz, auto&&... args)
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

/// @brief
/// @TODO(cava): current problem, since SoAReference is use on the fly often as pr-value, it lacks
/// the ability to move the elements of a SoAVec.
/// @tparam SV
template <inst_of<SoAVec> SV>
struct SoAReference {
    static_assert(inst_of<SV, SoAVec>);

    using self     = SoAReference;
    using tup_type = typename SV::value_type;

    int64_t idx;
    SV*     soa_vec;

    self& operator=(self const&& other) {
        generic_copy(other);
        return *this;
    }

    self& operator=(self&& other) {
        generic_copy(other);
        return *this;
    }

    self& operator=(auto&& tup) {
        generic_copy(tup);
        return *this;
    }

    // Explicit to permit assignment with {}-list
    self& operator=(tup_type const& tup) {
        generic_copy(tup);
        return *this;
    }

    void steal(self&& other) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (void(get<Is>(*this) = std::move(get<Is>(other))), ...);
        }(std::make_index_sequence<tup_type::size()>());
    }

private:
    void generic_copy(auto&& tup) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (void(get<Is>(*this) = get<Is>(tup)), ...);
        }(std::make_index_sequence<tup_type::size()>());
    }
};

template <std::size_t I, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV>& tup) {
    return get<I>(tup.soa_vec->base)[tup.idx];
}

template <std::size_t I, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV> const& tup) {
    return get<I>(tup.soa_vec->base)[tup.idx];
}

template <std::size_t I, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV>&& tup) {
    return get<I>(std::move(tup.soa_vec->base))[tup.idx];
}

template <typename T, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV>& tup) {
    return get<std::vector<T>>(tup.soa_vec->base)[tup.idx];
}

template <typename T, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV> const& tup) {
    return get<std::vector<T>>(tup.soa_vec->base)[tup.idx];
}

template <typename T, inst_of<SoAVec> SV>
[[nodiscard]] constexpr decl_auto get(SoAReference<SV>&& tup) {
    return get<std::vector<T>>(tup.soa_vec->base)[tup.idx];
}

template <typename>
struct SoAIterator;

template <typename... Ts>
class SoAVec<soa_tag, Ts...> {
public:
    using self                   = SoAVec;
    using value_type             = tuple<Ts...>;
    using pointer                = tuple<Ts*...>;
    using const_pointer          = tuple<Ts const*...>;
    using reference              = SoAReference<self>;
    using const_reference        = SoAReference<self const>;
    using iterator               = SoAIterator<self>;
    using const_iterator         = SoAIterator<self const>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;

    tuple<std::vector<Ts>...> base;

public:
    constexpr SoAVec() = default;

    constexpr SoAVec(size_type sz)
        : base{std::vector<Ts>(sz)...} {
    }

    constexpr SoAVec(size_type sz, auto&&... args)
    requires(sizeof...(args) == sizeof...(Ts))
        : base{std::vector<Ts>(sz, implicit_cast<Ts>(FWD(args)))...} {
    }

    constexpr SoAVec(size_type sz, auto const& tup)
        : base{apply(
              [&](auto const&... args) {
                  return tuple<std::vector<Ts>...>{std::vector<Ts>(sz, args)...};
              },
              tup)} {
    }

    template <typename InputIter>
    constexpr SoAVec(InputIter first, InputIter last) {
        insert(begin(), first, last);
    }

    constexpr void assign(size_type n, auto&&... args) {
        base.reduce([&](auto&... vs) { (vs.assign(n, FWD(args)), ...); });
    }

    constexpr void assign(size_type n, auto const& tup) {
        apply([&](auto const&... elems) { assign(n, elems...); }, tup);
    }

    // Element access
    [[nodiscard]] constexpr auto at(size_type idx) {
        return (*this)[idx];
    }

    [[nodiscard]] constexpr auto at(size_type idx) const {
        return (*this)[idx];
    }

    [[nodiscard]] constexpr SoAReference<self> operator[](int64_t idx) noexcept {
        assert(0 <= idx && idx < ssize(*this));
        return SoAReference<self>{idx, this};
    }

    [[nodiscard]] constexpr SoAReference<self const> operator[](int64_t idx) const noexcept {
        assert(0 <= idx && idx < ssize(*this));
        return SoAReference<self const>{idx, this};
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
        auto sz = base.first().size();
        assert((base.reduce([&](auto&... vs) { return ((sz == vs.size()) && ...); })));
        return sz;
    }

    [[nodiscard]] constexpr auto max_size() const {
        auto sz = base.first().max_size();
        assert((base.reduce([&](auto&... vs) { return ((sz == vs.max_size()) && ...); })));
        return sz;
    }

    constexpr void reserve(size_type sz) {
        base.reduce([&](auto&... vs) { (vs.reserve(sz), ...); });
    }

    [[nodiscard]] constexpr auto capacity() const {
        auto cap = base.first().capacity();
        assert((base.reduce([&](auto&... vs) { return ((cap == vs.capacity()) && ...); })));
        return cap;
    }

    // Modifiers
    constexpr void clear() {
        base.reduce([&](auto&... vs) { (vs.clear(), ...); });
    }

    constexpr iterator insert(const_iterator pos, value_type const& x) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).insert(get<Is>(pos), get<Is>(x)), ...);
        }(std::index_sequence_for<Ts...>{});
        return pos;
    }

    constexpr iterator insert(const_iterator pos, value_type&& x) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).insert(get<Is>(pos), get<Is>(std::move(x))), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    constexpr iterator insert(const_iterator pos, size_type n, value_type const& x) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).insert(get<Is>(pos), n, get<Is>(x)), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    template <typename InputIter>
    constexpr iterator insert(const_iterator pos, InputIter first, InputIter last) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).insert(get<Is>(pos), get<Is>(first), get<Is>(last)), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    constexpr iterator emplace(const_iterator pos, auto&&... args)
    requires(sizeof...(args) == sizeof...(Ts))
    {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).emplace(get<Is>(pos), args), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    constexpr iterator emplace(const_iterator pos, auto&& tup) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).emplace(get<Is>(pos), get<Is>(FWD(tup))), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    constexpr iterator erase(const_iterator pos) {

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).erase(get<Is>(pos)), ...);
        }(std::index_sequence_for<Ts...>{});
        return {pos.idx, this};
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((void)get<Is>(base).erase(get<Is>(first), get<Is>(last)), ...);
        }(std::index_sequence_for<Ts...>{});
        return {last.idx, this};
    }

    constexpr void push_back(value_type const& x) {
        x.reduce([&](auto const&... args) { emplace_back(args...); });
    }

    constexpr void push_back(value_type&& x) {
        std::move(x).reduce([&](auto&&... args) { emplace_back(FWD(args)...); });
    }

    constexpr reference emplace_back(auto&&... args)
    requires(sizeof...(args) <= sizeof...(Ts))
    {
        int64_t last = ssize(*this);

        constexpr int nargs = sizeof...(args);
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (void(get<Is>(base).emplace_back(FWD(args))), ...);
        }(std::make_index_sequence<nargs>{});

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (void(get<nargs + Is>(base).emplace_back()), ...);
        }(std::make_index_sequence<sizeof...(Ts) - nargs>{});

        return reference{last, this};
    }

    constexpr void pop_back() {
        base.reduce([&](auto&... vs) { (vs.pop_back(), ...); });
    }

    constexpr void resize(size_type sz) {
        base.reduce([&](auto&... vs) { (vs.resize(sz), ...); });
    }

    constexpr void swap(self& other) {
        base.swap(other.base);
    }
};

template <inst_of<SoAVec> SV>
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

    [[nodiscard]] constexpr bool operator!=(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx != other.idx;
    }

    [[nodiscard]] constexpr bool operator>(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx > other.idx;
    }

    [[nodiscard]] constexpr bool operator<(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx < other.idx;
    }

    [[nodiscard]] constexpr bool operator>=(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx >= other.idx;
    }

    [[nodiscard]] constexpr bool operator<=(self other) const {
        assert(soa_vec == other.soa_vec);
        return idx <= other.idx;
    }
};

template <inst_of<SoAVec> SV>
SoAIterator<SV> operator+(SoAIterator<SV> iter, ptrdiff_t n) {
    return iter += n;
}

template <inst_of<SoAVec> SV>
SoAIterator<SV> operator+(ptrdiff_t n, SoAIterator<SV> iter) {
    return iter += n;
}

template <inst_of<SoAVec> SV>
SoAIterator<SV> operator-(SoAIterator<SV> iter, ptrdiff_t n) {
    return iter -= n;
}

template <inst_of<SoAVec> SV>
SoAIterator<SV> operator-(ptrdiff_t n, SoAIterator<SV> iter) {
    return iter -= n;
}

template <std::size_t I, inst_of<SoAVec> SV>
auto get(SoAIterator<SV> iter) {
    return get<I>(iter.soa_vec->base).begin() + iter.idx;
}

static_assert(std::random_access_iterator<SoAIterator<SoAVec<soa_tag, int>>>);

/// @brief Utility to unwrap a variadic template into a SoAVec
template <typename tag, typename T>
struct soa_vec_from {
    using type = SoAVec<tag, T>;
};

template <typename tag, template <class...> class Tmpl, typename... Ts>
struct soa_vec_from<tag, Tmpl<Ts...>> {
    using type = SoAVec<tag, Ts...>;
};

template <typename tag, typename PackT>
using soa_vec_from_t = typename soa_vec_from<tag, PackT>::type;

template <typename SV1, typename SV2>
void swap(cav::SoAReference<SV1> r1, cav::SoAReference<SV2> r2) {
    using value_type = typename SV1::value_type;
    static_assert(std::same_as<value_type, typename SV2::value_type>);

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (std::swap(get<Is>(r1), get<Is>(r2)), ...);
    }(std::make_index_sequence<value_type::size()>{});
}
}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_SOAVEC_HPP */
