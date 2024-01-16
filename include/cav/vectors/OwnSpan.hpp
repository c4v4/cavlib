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
#include <concepts>
#include <memory>
#include <type_traits>

#include "../comptime/syntactic_sugars.hpp"
#include "../comptime/test.hpp"
#include "cav/comptime/mp_base.hpp"

namespace cav {

template <typename T>
struct ArrayDel {
    constexpr void operator()(T ptr[], size_t /*n*/) const noexcept {
        delete[] ptr;
    }
};

template <typename T>
struct AllocatorDel {
    constexpr void operator()(T ptr[], size_t n) const noexcept {
        std::destroy(ptr, ptr + n);
        std::allocator<T>{}.deallocate(ptr, n);
    }
};

/// @brief Like std::span but it owns the underlying pointer and frees the memory on destruction.
///  Uses-defined template-deduction-guides are used to automatically selected ArrayDel (i.e.,
///  delete[]) when the span is init with a pointer to T.
///  Otherwise, the dynamically sized array is allocated using std::allocator (I didn't want to
///  support custom allocators, the main use of this class should be for externally provided arrays,
///  and having to specify only how to release the memory is much simpler than having to defined a
///  completelly new allocator).
///  Finally, there is the pain in the neck of the non-default constructible types. In that case, a
///  functor is accepted to which the span is provided with the uninitialized memory already
///  allocated, ready to be "filled". This should allow quite a good deal of freedom.
///
/// @tparam T the value type of the span
/// @tparam Deleter the deleter invocable type, defaults to AllocatorDel<T>
template <typename T, class Deleter = AllocatorDel<T>>
class OwnSpan {
public:
    using self            = OwnSpan;
    using value           = T;
    using reference       = T&;
    using pointer         = T*;
    using const_reference = T const&;
    using const_pointer   = T const*;

    constexpr OwnSpan() = default;

    constexpr OwnSpan(std::integral auto c_size)
        : ptr(c_size > 0 ? _default_allocate(c_size) : nullptr)
        , sz(c_size) {
        static_assert(std::is_default_constructible_v<T>, "T is not default constructible.");
        assert(c_size >= 0);
        for (T& val : *this)
            std::construct_at(&val);
    }

    template <typename Fn>
    requires(std::invocable<Fn, self&> && !std::invocable<T, self&>)
    constexpr OwnSpan(std::integral auto c_size, Fn&& init)
        : ptr(c_size > 0 ? _default_allocate(c_size) : nullptr)
        , sz(c_size) {
        assert(c_size >= 0);
        FWD(init)(*this);
    }

    template <typename... Ts>
    constexpr OwnSpan(std::integral auto c_size, Ts const&... args)
        : ptr(c_size > 0 ? _default_allocate(c_size) : nullptr)
        , sz(c_size) {
        assert(c_size >= 0);
        for (T& val : *this)
            std::construct_at(&val, args...);
    }

    constexpr OwnSpan(T* c_ptr, std::integral auto c_size, Deleter const& c_del)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size)
        , del(c_del) {
    }

    constexpr OwnSpan(T* c_ptr, std::integral auto c_size, Deleter&& c_del)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size)
        , del(std::move(c_del)) {
    }

    constexpr OwnSpan(T* c_ptr, std::integral auto c_size)
        : ptr(c_size > 0 ? c_ptr : nullptr)
        , sz(c_size) {
    }

    constexpr OwnSpan(OwnSpan const& other)
        : ptr(other.sz > 0 ? _default_allocate(other.sz) : nullptr)
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
        if (std::addressof(other) == this)
            return *this;

        _free();
        ptr = other.sz > 0 ? new T[other.sz] : nullptr;
        sz  = other.sz;
        for (size_t i = 0; i < sz; ++i)
            ptr[i] = other[i];

        return *this;
    }

    constexpr OwnSpan& operator=(OwnSpan&& other) noexcept {
        if (std::addressof(other) == this)
            return *this;

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
    constexpr auto* _default_allocate(size_t n) {
        return std::allocator<T>{}.allocate(n);
    }

    constexpr void _free() {
        if (ptr != nullptr)
            del(ptr, sz);
        ptr = nullptr;
        sz  = 0;
    }

    T*                            ptr = {};
    size_t                        sz  = {};
    [[no_unique_address]] Deleter del = {};
};

// If init by extern ptr, assume default deleter
template <std::integral S, typename T>
OwnSpan(T*, S) -> OwnSpan<T, ArrayDel<T>>;

template <std::integral S, typename T>
OwnSpan(S, T) -> OwnSpan<T, AllocatorDel<T>>;


#ifdef CAV_COMP_TESTS
namespace test {

    static constexpr int  buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    static constexpr auto ospan   = OwnSpan(buff, 8, nop);  // equivalent to std::span

    // Test sizes
    CAV_PASS(16 == sizeof(OwnSpan<int>));
    CAV_PASS(16 == sizeof(OwnSpan<int, ArrayDel<int>>));
    CAV_PASS(32 == sizeof(OwnSpan(buff, 8, [i = .0L](auto&&...) { nop(i); })));  // 16byte deleter

    CAV_PASS(ospan.size() == 8);
    CAV_PASS(ospan[0] == 0);
    CAV_PASS(ospan[7] == 7);
    CAV_PASS(ospan.front() == 0);
    CAV_PASS(ospan.back() == 7);
    CAV_PASS(ospan.begin() == buff);
    CAV_PASS(ospan.end() == buff + 8);
    CAV_PASS(ospan.data() == buff);
    CAV_PASS(!ospan.empty());

    CAV_FAIL(ospan[7] == 8);    // ospan[7] == 7
    CAV_FAIL(ospan[8] == 0);    // out of bound (+1)
    CAV_FAIL(ospan[16] == 0);   // out of bound (+9)
    CAV_FAIL(ospan[-16] == 0);  // out of bound (-16)

    CAV_PASS(OwnSpan(new bool[10](), 1).back() == false);  // allocate 10, span of 1 -> Ok
    CAV_FAIL(OwnSpan(new bool[1](), 10).back() == false);  // allocate 1, span of 10 -> Error

    CAV_BLOCK_PASS({
        auto span2 = OwnSpan<bool>(8);
        for (bool& b : span2)
            b = true;
        span2.assign_all(false);

        span2.front() = span2.back() = true;
        span2[1] = span2[6] = true;
        assert(span2[0] != span2[2]);
    });

    // Test with non default constructible type
    CAV_PASS(OwnSpan<value_wrap<bool>>(1, [](auto& me) {
                 for (auto& s : me)
                     std::construct_at(&s, true);
             }).front());

    // Edge case where we have a span of invocables that accept the span itself. In that case
    // the custom initialization constructor cannot be called, since the custom init must be
    // considered as default value initialization.
    struct strange_invokable {
        constexpr void operator()(OwnSpan<strange_invokable>& me) {
            assert(me.empty());  // syntetic fail if invoked
            for (auto& s : me)
                std::construct_at(&s);
        }
    };

    CAV_BLOCK_PASS(OwnSpan<strange_invokable>(1, strange_invokable{}));  // Does not invoke-init
    CAV_BLOCK_FAIL({
        auto strange_span = OwnSpan<strange_invokable>{};
        OwnSpan<strange_invokable>{}.front()(strange_span);  // assert fail as expected
    });


}  // namespace test
#endif

}  // namespace cav

#endif /* CAV_BOUND_OWNSPAN_HPP */
