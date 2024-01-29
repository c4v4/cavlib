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

#ifndef CAV_INCLUDE_UTILS_OFFSETVEC_HPP
#define CAV_INCLUDE_UTILS_OFFSETVEC_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <vector>

#include "../comptime/mp_base.hpp"
#include "../comptime/test.hpp"
#include "cav/mish/util_functions.hpp"

namespace cav {

/// @brief OffsetVec: Simple wrapper around std::vector that allows offset access to its elements,
/// including negative indexes.
/// It supports efficient growth in both directions (amortized O(1) for both emplace_back and
/// emplace_front), heavily relying on std::vector for memory management.
/// To handle resizing at the front, which would normally require initialization of new elements, it
/// uses a simple union type that defers initialization, making it transparent to std::vector and
/// compatible with non-default constructible types.
/// A union is preferred over reinterpret_cast to enable usage in C++20 constexpr context.
///
/// TODO(cava): atm iterators rely on implicit conversion, I should implement a custom iterator type
template <typename T, typename AllocT = std::allocator<T>>
class OffsetVec {

    union maybe_uninit_wrap {
        cav::void_type uninit;
        T              value;

        static constexpr struct uninit_tag {
        } uninit_value = {};

        /// @brief Uninit ctor, it's your responsibility to manage its lifetime
        constexpr maybe_uninit_wrap(uninit_tag /*t*/)
            : uninit{} {
        }

        constexpr maybe_uninit_wrap(auto&&... args)
            : value{FWD(args)...} {
        }

        [[nodiscard]] constexpr operator T&() {
            return value;
        }

        [[nodiscard]] constexpr operator T const&() const {
            return value;
        }
    };

    using value_type          = T;
    using real_allocator_type = typename std::allocator_traits<AllocT>::template rebind_alloc<
        maybe_uninit_wrap>;

    using container_type  = std::vector<maybe_uninit_wrap, real_allocator_type>;
    using iterator        = typename container_type::iterator;
    using const_iterator  = typename container_type::const_iterator;
    using reference       = T&;
    using const_reference = T const&;
    using pointer         = T*;
    using const_pointer   = T const*;
    using size_type       = typename container_type::difference_type;
    using difference_type = typename container_type::difference_type;
    using allocator_type  = typename container_type::allocator_type;

    container_type vec       = {};
    iterator       offset_it = {};
    iterator       beg_it    = {};

public:
    constexpr OffsetVec()                                = default;
    constexpr OffsetVec(OffsetVec&&) noexcept            = default;
    constexpr OffsetVec& operator=(OffsetVec&&) noexcept = default;

    constexpr OffsetVec(size_type c_offset, auto&& arg, auto&&... args)
        : vec(FWD(arg), FWD(args)...)
        , offset_it(vec.begin() + c_offset)
        , beg_it(vec.begin()) {
        assert(0 <= c_offset && c_offset < static_cast<size_type>(vec.size()));
    }

    template <typename... Args>
    constexpr OffsetVec(size_type                                c_offset,
                        std::initializer_list<maybe_uninit_wrap> l,
                        Args&&... args)
        : vec(l, FWD(args)...)
        , offset_it(vec.begin() + c_offset)
        , beg_it(vec.begin()) {
        assert(0 <= c_offset && c_offset < static_cast<size_type>(vec.size()));
    }

    constexpr OffsetVec(OffsetVec const& other)
        : vec(other.vec)
        , offset_it(vec.begin() + (other.offset_it - other.vec.begin()))
        , beg_it(vec.begin() + (other.beg_it - other.vec.begin())) {
    }

    constexpr OffsetVec& operator=(OffsetVec const& other) {
        vec       = other.vec;
        offset_it = vec.begin() + (other.offset_it - other.vec.begin());
        beg_it    = vec.begin() + (other.beg_it - other.vec.begin());
    }

    constexpr ~OffsetVec() {
        for (auto& elem : *this)
            std::destroy_at(std::addressof(elem.value));
    }

    [[nodiscard]] constexpr size_type get_offset() const {
        return offset_it - beg_it;
    }

    constexpr size_type set_offset(size_type new_offset) {
        assert(new_offset < size());
        auto old_offset = offset_it;
        offset_it       = beg_it + new_offset;
        return offset_it - old_offset;
    }

    [[nodiscard]] constexpr reference operator[](difference_type n) {
        assert(beg_idx() <= n && n < end_idx());
        return offset_it[n].value;
    }

    [[nodiscard]] constexpr const_reference operator[](difference_type n) const {
        assert(beg_idx() <= n && n < end_idx());
        return offset_it[n].value;
    }

    [[nodiscard]] constexpr reference front() {
        return beg_it->value;
    }

    [[nodiscard]] constexpr const_reference front() const {
        return beg_it->value;
    }

    [[nodiscard]] constexpr reference back() {
        return vec.back().value;
    }

    [[nodiscard]] constexpr const_reference back() const {
        return vec.back().value;
    }

    [[nodiscard]] constexpr iterator begin() {
        return beg_it;
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return beg_it;
    }

    [[nodiscard]] constexpr iterator mid() {
        return offset_it;
    }

    [[nodiscard]] constexpr const_iterator mid() const {
        return offset_it;
    }

    [[nodiscard]] constexpr iterator end() {
        return vec.end();
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return vec.end();
    }

    [[nodiscard]] constexpr size_type size() const {
        return end() - begin();
    }

    [[nodiscard]] constexpr bool empty() const {
        return end() == begin();
    }

    [[nodiscard]] constexpr difference_type beg_idx() const {
        return begin() - mid();
    }

    [[nodiscard]] constexpr difference_type end_idx() const {
        return end() - mid();
    }

    [[nodiscard]] constexpr size_type capacity() const {
        return vec.capacity();
    }

    [[nodiscard]] constexpr size_type front_space() const {
        assert(beg_it - vec.begin() >= 0);
        return beg_it - vec.begin();
    }

    constexpr void make_space_front(size_type n) {
        if (n > front_space()) {
            size_type offset = get_offset(), beg_offset = n - front_space();
            vec.insert(vec.begin(), beg_offset, maybe_uninit_wrap{maybe_uninit_wrap::uninit_value});
            beg_it    = vec.begin() + beg_offset;
            offset_it = beg_it + offset;
        }
    }

    constexpr reference emplace_front(auto&&... args) {
        if (front_space() == 0)
            make_space_front(cav::max(4, (size() + 1) / 2));
        --beg_it;
        // no need to destroy, trivial dtor
        std::construct_at(std::addressof(beg_it->value), FWD(args)...);
        return beg_it->value;
    }

    constexpr void push_front(auto&& arg) {
        if (!std::is_constant_evaluated() && vec.capacity() > 0)  // alias check
            assert(std::less<>{}(&arg, &beg_it->value) ||
                   std::greater_equal{}(&arg, &end()->value));
        emplace_front(FWD(arg));
    }

    constexpr void pop_front() {
        std::destroy_at(&beg_it->val);
        ++beg_it;
    }

    [[nodiscard]] constexpr size_type back_space() const {
        assert(vec.capacity() - vec.size() >= 0);
        return vec.capacity() - vec.size();
    }

    constexpr void make_space_back(size_type n) {
        if (n > back_space()) {
            size_type offset = get_offset(), beg_offset = front_space();
            vec.reserve(vec.size() + n);
            beg_it    = vec.begin() + beg_offset;
            offset_it = beg_it + offset;
        }
    }

    constexpr reference emplace_back(auto&&... args) {
        if (back_space() <= 0)
            make_space_back(cav::max(4, (size() + 1) / 2));
        return vec.emplace_back(FWD(args)...).value;
    }

    constexpr void push_back(auto&& arg) {
        if (!std::is_constant_evaluated() && vec.capacity() > 0)  // alias check
            assert(std::less<>{}(&arg, &beg_it->value) ||
                   std::greater_equal{}(&arg, &end()->value));
        emplace_back(FWD(arg));
    }

    constexpr void pop_back() {
        vec.pop_back();
    }
};
}  // namespace cav

namespace cav {

#ifdef CAV_COMP_TESTS

namespace {

    // Test sizes
    CAV_PASS(40 == sizeof(OffsetVec<int>));

#define CAV_TEST_OFFVEC_3_8 (OffsetVec<int>(3, {0, 1, 2, 3, 4, 5, 6, 7}))
    CAV_PASS(CAV_TEST_OFFVEC_3_8.size() == 8);

    CAV_PASS(CAV_TEST_OFFVEC_3_8.size() == 8);
    CAV_PASS(CAV_TEST_OFFVEC_3_8.front() == 0);
    CAV_PASS(CAV_TEST_OFFVEC_3_8.back() == 7);
    CAV_PASS(!CAV_TEST_OFFVEC_3_8.empty());

    CAV_FAIL(CAV_TEST_OFFVEC_3_8[-4] == 0);  // OOB
    CAV_PASS(CAV_TEST_OFFVEC_3_8[-3] == 0);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[-2] == 1);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[-1] == 2);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[0] == 3);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[1] == 4);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[2] == 5);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[3] == 6);
    CAV_PASS(CAV_TEST_OFFVEC_3_8[4] == 7);
    CAV_FAIL(CAV_TEST_OFFVEC_3_8[5] == 7);  // OOB
#undef CAV_TEST_OFFVEC_3_8


    CAV_BLOCK_PASS({
        auto ovec = OffsetVec<bool>(2, 7);
        // Sizes check
        assert(!ovec.empty());
        assert(ovec.size() == 7);
        assert(ovec.capacity() == 7);
        assert(ovec.front_space() == 0);
        assert(ovec.back_space() == 0);

        for (bool& b : ovec)
            b = true;

        // Access check
        ovec.front() = ovec.back() = false;
        ovec[1] = ovec[4] = false;
        assert(ovec[-2] != ovec[2]);
        assert(ovec[-2] || ovec[2]);
        assert(ovec[-2] + ovec[2] == true);

        // Access check with different offset
        ovec.set_offset(6);
        assert(ovec[-6] == false);
        assert(ovec[-5] == true);
        assert(ovec[-4] == true);
        assert(ovec[-3] == false);
        assert(ovec[-2] == true);
        assert(ovec[-1] == true);
        assert(ovec[0] == false);
    });

    CAV_BLOCK_PASS({
        auto ovec = OffsetVec<bool>(6, 7);

        // Element insertion (conservative checks, cannot assume std::vector internals)
        ovec.emplace_front();  // cap=14, frt_space=3, bck_space=4
        assert(ovec.capacity() >= 11 && ovec.front_space() == 3);
        ovec.emplace_front(true);  // cap=14, frt_space=2, bck_space=4
        assert(ovec.capacity() >= 11 && ovec.front_space() == 2);
        ovec.emplace_front(false);  // cap=14, frt_space=1, bck_space=4
        assert(ovec.capacity() >= 11 && ovec.front_space() == 1);

        ovec.emplace_back();  // cap=14, frt_space=1, bck_space=2
        assert(ovec.capacity() >= 12 && ovec.front_space() == 1);
        ovec.emplace_back(true);  // cap=14, frt_space=1, bck_space=1
        assert(ovec.capacity() >= 13 && ovec.front_space() == 1);
        ovec.emplace_back(false);  // cap=14, frt_space=1, bck_space=0
        assert(ovec.capacity() >= 14 && ovec.front_space() == 1);

        assert(ovec[-9] == false);
        assert(ovec[-8] == true);
        assert(ovec[-7] == false);
        assert(ovec[1] == false);
        assert(ovec[2] == true);
        assert(ovec[3] == false);
    });

    CAV_BLOCK_PASS({
        auto ovec = OffsetVec<bool>();
        ovec.push_front(true);  // resize +4
        assert(ovec.capacity() >= 4 && ovec.front_space() == 3);
        ovec.push_front(false);
        assert(ovec.capacity() >= 4 && ovec.front_space() == 2);
        ovec.push_front(false);
        assert(ovec.capacity() >= 4 && ovec.front_space() == 1);
        ovec.push_front(false);
        assert(ovec.capacity() >= 4 && ovec.front_space() == 0);
        ovec.push_front(false);  // resize +4
        assert(ovec.capacity() >= 8 && ovec.front_space() == 3);

        ovec.push_back(false);  // resize size()/2 == 4
        assert(ovec.capacity() >= 12 && ovec.back_space() == 3);
        ovec.push_back(true);
        assert(ovec.capacity() >= 12 && ovec.back_space() == 2);
        ovec.push_back(false);
        assert(ovec.capacity() >= 12 && ovec.back_space() == 1);
        ovec.push_back(false);
        assert(ovec.capacity() >= 12 && ovec.back_space() == 0);
        ovec.push_back(false);  // resize (size()+1)/2 == 5
        assert(ovec.capacity() >= 17 && ovec.back_space() == 4);

        ovec.set_offset(3);
        assert(ovec[0] == false);
        assert(ovec[1] == true);
        assert(ovec[2] == false);
        assert(ovec[3] == true);
    });

}  // namespace
#endif

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_OFFSETVEC_HPP */
