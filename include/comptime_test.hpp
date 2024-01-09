// Copyright (c) 2024 Francesco Cavaliere
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

#ifndef CAV_INCLUDE_COMPTIME_TEST_HPP
#define CAV_INCLUDE_COMPTIME_TEST_HPP

/// @brief Return true (i.e., expression fails) if "E{}()" evaluates to false or if it is not a
/// costant expression. Adapted from https://stackoverflow.com/a/55290363
namespace cav::test_detail {
template <typename E, bool R = E{}()>
constexpr bool expr_fails(E) {
    return !R;
}

constexpr bool expr_fails(...) {
    return true;
}
}  // namespace cav::test_detail

/// @brief Basic static assert, pass if expression is true at compile time
#define CAV_PASS(...) static_assert((__VA_ARGS__));

/// @brief Opposite case, pass if expression is false or if it is not constexpr
#define CAV_FAIL(...) \
    static_assert(::cav::test_detail::expr_fails([] { return static_cast<bool>(__VA_ARGS__); }))

/// @brief Adapter for code blocks, pass if it is a valid constexpr block
#define CAV_BLOCK_PASS(...) \
    CAV_PASS([] {           \
        __VA_ARGS__;        \
        return true;        \
    }());

/// @brief Opposite case, pass if it is not a valid constexpr block (e.g. assert fail or UB)
#define CAV_BLOCK_FAIL(...) \
    CAV_FAIL([] {           \
        __VA_ARGS__;        \
        return true;        \
    }());


#ifdef COMP_TESTS
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// TEST IF IT WORKS AS EXPECTED ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <array>
#include <cassert>
#include <string>
#include <vector>

namespace cav::test {
constexpr auto fail_assert_ret_true = []() {
    assert(false);
    return true;
};

constexpr auto invoke_ub = []() {
    bool* p = nullptr;
    return (*p = true);
};

CAV_PASS(true);                   // base case
CAV_PASS(std::array{true}[0]);    // Constexpr expression -> ok
CAV_PASS(std::true_type{});       // Constexpr true -> ok
CAV_PASS([] { return true; }());  // Lambda return true -> ok

CAV_FAIL(false);                   // base case
CAV_FAIL(*(new bool()));           // No delete at constexpr -> fail
CAV_FAIL(std::false_type{});       // Constexpr false -> fail
CAV_FAIL(fail_assert_ret_true());  // Assert false -> fail
CAV_FAIL(invoke_ub());             // Undefined behavior -> fail
CAV_FAIL([] { return false; }());  // Lambda return false -> fail

CAV_BLOCK_PASS({
    auto v = std::vector<int>(4);
    for (int i = 0; i < 4; ++i)
        v[i] = i;
});

CAV_BLOCK_FAIL({
    auto v = std::vector<int>(4);
    for (int i = 0; i <= /*off-by-one*/ 4; ++i)
        v[i] = i;
});

CAV_BLOCK_PASS({
    auto str    = std::string{"This test should pass"};
    auto last_w = std::string_view{str}.substr(str.find_last_of(' ') + 1);
    assert(last_w == "pass");
});

CAV_BLOCK_FAIL({
    auto str    = std::string{"This test should fail"};
    auto last_w = std::string_view{str}.substr(str.find_last_of(' ') + 1);
    assert(last_w == "pass");
});
}  // namespace cav::test
#endif

#endif /* CAV_INCLUDE_COMPTIME_TEST_HPP */
