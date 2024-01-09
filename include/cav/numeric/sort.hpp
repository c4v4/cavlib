// Copyright (c) 2022 Francesco Cavaliere
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

#ifndef CAV_INCLUDE_UTILS_SORT_HPP
#define CAV_INCLUDE_UTILS_SORT_HPP


#include <functional>

#include "../mish/functors.hpp"
#include "../numeric/sorting_networks.hpp"

namespace cav {

template <typename It, typename ExtractKey>
static void sort(It begin, It end, ExtractKey&& extract_key) {
    // ska::sort(begin, end, extract_key);
    std::sort(begin, end, [&](auto const& x, auto const& y) {
        return extract_key(x) < extract_key(y);
    });
}

template <typename It>
static void sort(It begin, It end) {
    // ska::sort(begin, end);
    std::sort(begin, end);
}

template <typename It, typename CompT = less_ftor>
static void small_sort(It beg, It end, CompT&& comp = {}) {
    int size = end - beg;
    switch (size) {
    case 2:
        netsort::s2(beg, comp);
        break;
    case 3:
        netsort::s3(beg, comp);
        break;
    case 4:
        netsort::s4(beg, comp);
        break;
    case 5:
        netsort::s5(beg, comp);
        break;
    case 6:
        netsort::s6(beg, comp);
        break;
    case 7:
        netsort::s7(beg, comp);
        break;
    case 8:
        netsort::s8(beg, comp);
        break;
    case 9:
        netsort::s9(beg, comp);
        break;
    case 10:
        netsort::s10(beg, comp);
        break;
    case 11:
        netsort::s11(beg, comp);
        break;
    case 12:
        netsort::s12(beg, comp);
        break;
    case 13:
        netsort::s13(beg, comp);
        break;
    case 14:
        netsort::s14(beg, comp);
        break;
    case 15:
        netsort::s15(beg, comp);
        break;
    case 16:
        netsort::s16(beg, comp);
        break;
    case 17:
        netsort::s17(beg, comp);
        break;
    case 18:
        netsort::s18(beg, comp);
        break;
    case 19:
        netsort::s19(beg, comp);
        break;
    case 20:
        netsort::s20(beg, comp);
        break;
    case 21:
        netsort::s21(beg, comp);
        break;
    case 22:
        netsort::s22(beg, comp);
        break;
    case 23:
        netsort::s23(beg, comp);
        break;
    case 24:
        netsort::s24(beg, comp);
        break;
    case 25:
        netsort::s25(beg, comp);
        break;
    case 26:
        netsort::s26(beg, comp);
        break;
    case 27:
        netsort::s27(beg, comp);
        break;
    case 28:
        netsort::s28(beg, comp);
        break;
    case 29:
        netsort::s29(beg, comp);
        break;
    case 30:
        netsort::s30(beg, comp);
        break;
    case 31:
        netsort::s31(beg, comp);
        break;
    case 32:
        netsort::s32(beg, comp);
        break;
    default:
        std::sort(beg, end, [&](auto n1, auto n2) { return comp(n1, n2); });
    }
}

/// @brief Network sorting for small cases. Since the size is known at compile time,
/// cases up to 32 are included (and discarded by the compiler if not used).
///
template <int Sz, typename ContainerT, typename CompT = less_ftor>
static void small_sort(ContainerT& array, CompT&& comp = {}) {
    if constexpr (Sz == 2)
        netsort::s2(std::begin(array), comp);
    else if constexpr (Sz == 3)
        netsort::s3(std::begin(array), comp);
    else if constexpr (Sz == 4)
        netsort::s4(std::begin(array), comp);
    else if constexpr (Sz == 5)
        netsort::s5(std::begin(array), comp);
    else if constexpr (Sz == 6)
        netsort::s6(std::begin(array), comp);
    else if constexpr (Sz == 7)
        netsort::s7(std::begin(array), comp);
    else if constexpr (Sz == 8)
        netsort::s8(std::begin(array), comp);
    else if constexpr (Sz == 9)
        netsort::s9(std::begin(array), comp);
    else if constexpr (Sz == 10)
        netsort::s10(std::begin(array), comp);
    else if constexpr (Sz == 11)
        netsort::s11(std::begin(array), comp);
    else if constexpr (Sz == 12)
        netsort::s12(std::begin(array), comp);
    else if constexpr (Sz == 13)
        netsort::s13(std::begin(array), comp);
    else if constexpr (Sz == 14)
        netsort::s14(std::begin(array), comp);
    else if constexpr (Sz == 15)
        netsort::s15(std::begin(array), comp);
    else if constexpr (Sz == 16)
        netsort::s16(std::begin(array), comp);
    else if constexpr (Sz == 17)
        netsort::s17(std::begin(array), comp);
    else if constexpr (Sz == 18)
        netsort::s18(std::begin(array), comp);
    else if constexpr (Sz == 19)
        netsort::s19(std::begin(array), comp);
    else if constexpr (Sz == 20)
        netsort::s20(std::begin(array), comp);
    else if constexpr (Sz == 21)
        netsort::s21(std::begin(array), comp);
    else if constexpr (Sz == 22)
        netsort::s22(std::begin(array), comp);
    else if constexpr (Sz == 23)
        netsort::s23(std::begin(array), comp);
    else if constexpr (Sz == 24)
        netsort::s24(std::begin(array), comp);
    else if constexpr (Sz == 25)
        netsort::s25(std::begin(array), comp);
    else if constexpr (Sz == 26)
        netsort::s26(std::begin(array), comp);
    else if constexpr (Sz == 27)
        netsort::s27(std::begin(array), comp);
    else if constexpr (Sz == 28)
        netsort::s28(std::begin(array), comp);
    else if constexpr (Sz == 29)
        netsort::s29(std::begin(array), comp);
    else if constexpr (Sz == 30)
        netsort::s30(std::begin(array), comp);
    else if constexpr (Sz == 31)
        netsort::s31(std::begin(array), comp);
    else if constexpr (Sz == 32)
        netsort::s32(std::begin(array), comp);
    else if constexpr (Sz >= 33)
        std::sort(std::begin(array), std::end(array), [&](auto const& n1, auto const& n2) {
            return comp(n1, n2);
        });
}

template <typename ContainerT, typename CompT = less_ftor>
static void small_sort(ContainerT& container, CompT&& comp = {}) {
    return small_sort(container.begin(), container.end(), FWD(comp));
}

template <typename T, size_t Sz, typename CompT = less_ftor>
static void small_sort(std::array<T, Sz>& array, CompT&& comp = {}) {
    return small_sort<Sz>(array, FWD(comp));
}

}  // namespace cav


#endif /* CAV_INCLUDE_UTILS_SORT_HPP */
