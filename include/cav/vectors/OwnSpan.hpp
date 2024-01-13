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

#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"

namespace cav {

template <typename T>
struct ArrayDel {
    constexpr void operator()(T ptr[]) const noexcept {
        delete[] ptr;
    }
};

/// @brief Like std::span but it owns the underlying pointer and frees the memory on destruction
/// @tparam T
template <typename T, class Deleter = ArrayDel<T>>
class OwnSpan {
public:
    using self            = OwnSpan;
    using value           = T;
    using reference       = T&;
    using pointer         = T*;
    using const_reference = T const&;
    using const_pointer   = T const*;

    constexpr OwnSpan() = default;

    template <std::integral S, typename U = T, typename... Ts>
    constexpr OwnSpan(S c_size, U const& first = {}, Ts const&... args)
        : ptr(c_size > 0 ? std::allocator<T>{}.allocate(c_size) : nullptr)
        , sz(c_size) {
        assert(c_size >= 0);
        for (T& val : *this)
            std::construct_at(&val, first, args...);
    }

    constexpr OwnSpan(T* c_ptr, size_t c_size, Deleter const& c_del)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size)
        , del(c_del) {
    }

    constexpr OwnSpan(T* c_ptr, size_t c_size, Deleter&& c_del)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size)
        , del(std::move(c_del)) {
    }

    constexpr OwnSpan(T* c_ptr, size_t c_size)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size) {
    }

    constexpr OwnSpan(OwnSpan const& other)
        : ptr(other.sz > 0 ? std::allocator<T>{}.allocate(other.sz) : nullptr)
        , sz(other.sz) {
        for (size_t i = 0; i < sz; ++i)
            std::construct_at(ptr + i, other[i]);
    }

    constexpr OwnSpan(OwnSpan&& other) noexcept
        : ptr(other.ptr)
        , sz(other.sz) {
        other.ptr = nullptr;
        other.sz  = 0;
    }

    constexpr OwnSpan& operator=(OwnSpan const& other) {
        if (&other == this)
            return *this;

        _free();
        ptr = other.sz > 0 ? new T[other.sz] : nullptr;
        sz  = other.sz;
        for (size_t i = 0; i < sz; ++i)
            ptr[i] = other[i];

        return *this;
    }

    constexpr OwnSpan& operator=(OwnSpan&& other) noexcept {
        _free();
        std::swap(ptr, other.ptr);
        std::swap(sz, other.sz);
        return *this;
    }

    constexpr ~OwnSpan() {
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

    [[nodiscard]] constexpr pointer begin() {
        return ptr;
    }

    [[nodiscard]] constexpr const_pointer begin() const {
        return ptr;
    }

    [[nodiscard]] constexpr pointer end() {
        return ptr + sz;
    }

    [[nodiscard]] constexpr const_pointer end() const {
        return ptr + sz;
    }

    [[nodiscard]] constexpr size_t size() const {
        return sz;
    }

    [[nodiscard]] constexpr bool empty() const {
        assert((ptr == nullptr) == (sz == 0));
        return sz == 0;
    }

    [[nodiscard]] constexpr reference front() {
        return ptr[0];
    }

    [[nodiscard]] constexpr const_reference front() const {
        return ptr[0];
    }

    [[nodiscard]] constexpr reference back() {
        return ptr[sz - 1];
    }

    [[nodiscard]] constexpr const_reference back() const {
        return ptr[sz - 1];
    }

    [[nodiscard]] constexpr reference operator[](size_t i) {
        assert(i < size());
        return ptr[i];
    }

    [[nodiscard]] constexpr const_reference operator[](size_t i) const {
        assert(i < size());
        return ptr[i];
    }

    constexpr void assign_all(T const& val) noexcept {
        for (T& v : *this)
            v = val;
    }

private:
    constexpr void _free() {
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

#ifdef CAV_COMP_TESTS
namespace test {

    static constexpr int  buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    static constexpr auto span    = OwnSpan(buff, 8, nop);

    CAV_PASS(span.size() == 8);
    CAV_PASS(span[0] == 0);
    CAV_PASS(span[7] == 7);
    CAV_PASS(span.front() == 0);
    CAV_PASS(span.back() == 7);
    CAV_PASS(span.begin() == buff);
    CAV_PASS(span.end() == buff + 8);
    CAV_PASS(span.data() == buff);
    CAV_PASS(!span.empty());

    CAV_FAIL(span[7] == 8);
    CAV_FAIL(span[8] == 0);
    CAV_FAIL(span[16] == 0);
    CAV_FAIL(span[-16] == 0);

    CAV_BLOCK_PASS({
        auto span2 = OwnSpan<bool>(8);
        for (bool& b : span2)
            b = true;
        span2.assign_all(false);

        span2.front() = span2.back() = true;
        span2[1] = span2[6] = true;
        return span2[0] != span2[2];
    });

}  // namespace test
#endif

}  // namespace cav

#endif /* CAV_BOUND_OWNSPAN_HPP */
