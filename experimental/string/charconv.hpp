#ifndef CAV_INCLUDE_UTILS_CHARCONV_HPP
#define CAV_INCLUDE_UTILS_CHARCONV_HPP

#include <charconv>
#include <concepts>

#include "../numeric/ScaledInt.hpp"
#include "../numeric/TolFloat.hpp"

namespace std {

template <int8_t E, int64_t B, std::integral BT, cav::rounding_tag RT>
auto from_chars(char const*                   first,
                char const*                   last,
                cav::ScaledInt<E, B, BT, RT>& value,
                chars_format                  fmt = chars_format::general) {
    double val{};
    auto   res = std::from_chars(first, last, val, fmt);
    value.from_val(val);
    return res;
}

template <int8_t E, int64_t B, std::floating_point BT>
auto from_chars(char const*              first,
                char const*              last,
                cav::TolFloat<E, B, BT>& value,
                chars_format             fmt = chars_format::general) {
    double val{};
    auto   res = std::from_chars(first, last, val, fmt);
    value.from_val(val);
    return res;
}

}  // namespace std

#endif /* CAV_INCLUDE_UTILS_CHARCONV_HPP */
