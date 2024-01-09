#ifndef CAV_INCLUDE_VECTORKD_HPP
#define CAV_INCLUDE_VECTORKD_HPP

#include <vector>

namespace cav {
namespace detail {
    template <typename T, int K>
    struct make_vectorKD {
        using type = std::vector<typename make_vectorKD<T, K - 1>::type>;
    };

    template <typename T>
    struct make_vectorKD<T, 0> {
        using type = T;
    };
}  // namespace detail

template <typename T, int K>
using VectorKD = typename detail::make_vectorKD<T, K>::type;

}  // namespace cav

#endif /* CAV_INCLUDE_VECTORKD_HPP */
