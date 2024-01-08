// Copyright (c) 2023 Francesco Cavaliere
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

#ifndef CAV_BOUND_OWNSPAN_HPP
#define CAV_BOUND_OWNSPAN_HPP

#include <cassert>
#include <memory>
#include <type_traits>

namespace cav {

/// @brief Like std::span but it owns the underlying pointer and frees the memory on destruction
/// @tparam T
template <typename T, class Deleter = std::default_delete<T[]>>
class OwnSpan {
public:
    using self            = OwnSpan;
    using value           = T;
    using reference       = T&;
    using pointer         = T*;
    using const_reference = T const&;
    using const_pointer   = T const*;

    OwnSpan() = default;

    template <std::integral S, typename U = T, typename... Ts>
    OwnSpan(S c_size, U const& first = {}, Ts const&... args)
        : ptr(c_size > 0 ? new T[c_size] : nullptr)
        , sz(c_size) {
        assert(c_size >= 0);
        for (T& val : *this)
            std::construct_at(&val, first, args...);
    }

    OwnSpan(T* c_ptr, size_t c_size, Deleter&& c_del)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size)
        , del(std::move(c_del)) {
    }

    OwnSpan(T* c_ptr, size_t c_size)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size) {
    }

    OwnSpan(OwnSpan const& other)
        : ptr(other.sz > 0 ? new T[other.sz] : nullptr)
        , sz(other.sz) {
        for (size_t i = 0; i < sz; ++i)
            std::construct_at(ptr + i, other[i]);
    }

    OwnSpan(OwnSpan&& other) noexcept
        : ptr(other.ptr)
        , sz(other.sz) {
        other.ptr = nullptr;
        other.sz  = 0;
    }

    OwnSpan& operator=(OwnSpan const& other) {
        if (&other == this)
            return *this;

        _free();
        ptr = other.sz > 0 ? new T[other.sz] : nullptr;
        sz  = other.sz;
        for (size_t i = 0; i < sz; ++i)
            ptr[i] = other[i];

        return *this;
    }

    OwnSpan& operator=(OwnSpan&& other) noexcept {
        _free();
        std::swap(ptr, other.ptr);
        std::swap(sz, other.sz);
        return *this;
    }

    ~OwnSpan() {
        _free();
    }

    [[nodiscard]] constexpr operator T*() {
        return ptr;
    }

    [[nodiscard]] constexpr operator T const*() const {
        return ptr;
    }

    [[nodiscard]] constexpr T* data() {
        return ptr;
    }

    [[nodiscard]] constexpr T const* data() const {
        return ptr;
    }

    [[nodiscard]] pointer begin() {
        return ptr;
    }

    [[nodiscard]] const_pointer begin() const {
        return ptr;
    }

    [[nodiscard]] pointer end() {
        return ptr + sz;
    }

    [[nodiscard]] const_pointer end() const {
        return ptr + sz;
    }

    [[nodiscard]] size_t size() const {
        return sz;
    }

    [[nodiscard]] bool empty() const {
        assert((ptr == nullptr) == (sz == 0));
        return sz == 0;
    }

    [[nodiscard]] reference front() {
        return ptr[0];
    }

    [[nodiscard]] const_reference front() const {
        return ptr[0];
    }

    [[nodiscard]] reference back() {
        return ptr[sz - 1];
    }

    [[nodiscard]] const_reference back() const {
        return ptr[sz - 1];
    }

    [[nodiscard]] reference operator[](size_t i) {
        assert(i < size());
        return ptr[i];
    }

    [[nodiscard]] const_reference operator[](size_t i) const {
        assert(i < size());
        return ptr[i];
    }

    void assign_all(T const& val) noexcept {
        for (T& v : *this)
            v = val;
    }

private:
    void _free() {
        if (ptr != nullptr)
            del(ptr);
        ptr = nullptr;
        sz  = 0;
    }

    T*                            ptr = {};
    size_t                        sz  = {};
    [[no_unique_address]] Deleter del = {};
};

template <std::integral S, typename T>
OwnSpan(S, T) -> OwnSpan<T>;

}  // namespace cav

#endif /* CAV_BOUND_OWNSPAN_HPP */
