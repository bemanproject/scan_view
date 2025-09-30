// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <algorithm>
#include <functional>
#include <vector>

#include <gtest/gtest.h>

#include <beman/scan_view/scan.hpp>

namespace exe = beman::scan_view;

template <class E1, class E2, std::size_t N, class Join = std::plus<E1>>
auto joinArrays(E1 (&a)[N], E2 (&b)[N], Join join = Join()) {
    return exe::scan(a, [&a, &b, join](auto sum, auto& cur) {
        auto idx = (&cur) - a;
        return join(sum, b[idx]);
    });
}

struct NonConstView : std::ranges::view_base {
    explicit NonConstView(int* b, int* e) : b_(b), e_(e) {}
    const int* begin() { return b_; } // deliberately non-const
    const int* end() { return e_; }   // deliberately non-const
    const int* b_;
    const int* e_;
};

struct NonTrivialFunctor {
    int           var_;
    constexpr int operator()(int a, int b) const { return a + b; }
};

// Some basic examples of how transform_view might be used in the wild. This is a general
// collection of sample algorithms and functions that try to mock general usage of
// this view.

TEST(ScanView, General) {
    std::vector<int> vec         = {1, 2, 3, 4};
    auto             transformed = exe::scan(vec, std::plus{});
    int              expected[]  = {1, 3, 6, 10};
    ASSERT_TRUE(std::ranges::equal(transformed, expected));
    const auto& ct = transformed;
    ASSERT_TRUE(std::ranges::equal(ct, expected));

    auto transformed2 = exe::scan(vec, std::plus{}, 10);
    int  expected2[]  = {11, 13, 16, 20};
    ASSERT_TRUE(std::ranges::equal(transformed2, expected2));
    const auto& ct2 = transformed2;
    ASSERT_TRUE(std::ranges::equal(ct2, expected2));
}

TEST(ScanView, Properties) {
    std::vector<int> vec         = {1, 2, 3, 4};
    auto             transformed = exe::scan(vec, std::plus{});
    static_assert(std::is_same_v<std::ranges::range_value_t<decltype(transformed)>, int>);
    static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(transformed)>, const int&>);
    static_assert(std::ranges::forward_range<decltype(transformed)>);
    static_assert(!std::ranges::common_range<decltype(transformed)>);
    static_assert(std::ranges::sized_range<decltype(transformed)>);
    static_assert(std::ranges::range<const decltype(transformed)>);
    static_assert(std::ranges::borrowed_range<decltype(transformed)>);
    ASSERT_EQ(transformed.size(), 4);

    auto transformed2 = exe::scan(vec, std::plus{}, 10);
    static_assert(std::is_same_v<std::ranges::range_value_t<decltype(transformed2)>, int>);
    static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(transformed2)>, const int&>);
    static_assert(std::ranges::forward_range<decltype(transformed2)>);
    static_assert(!std::ranges::common_range<decltype(transformed2)>);
    static_assert(std::ranges::sized_range<decltype(transformed2)>);
    static_assert(std::ranges::range<const decltype(transformed2)>);
    static_assert(std::ranges::borrowed_range<decltype(transformed2)>);
    ASSERT_EQ(transformed2.size(), 4);

    auto transformed3 = exe::scan(vec, NonTrivialFunctor{});
    static_assert(std::is_same_v<std::ranges::range_value_t<decltype(transformed3)>, int>);
    static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(transformed3)>, const int&>);
    static_assert(std::ranges::forward_range<decltype(transformed3)>);
    static_assert(!std::ranges::common_range<decltype(transformed3)>);
    static_assert(std::ranges::sized_range<decltype(transformed3)>);
    static_assert(std::ranges::range<const decltype(transformed3)>);
    static_assert(!std::ranges::borrowed_range<decltype(transformed3)>);
    ASSERT_EQ(transformed3.size(), 4);
}

TEST(ScanView, NonConstIterable) {
    // Test a view type that is not const-iterable.
    int  a[]         = {1, 2, 3, 4};
    auto transformed = NonConstView(a, a + 4) | exe::scan(std::plus{});
    int  expected[]  = {1, 3, 6, 10};
    ASSERT_TRUE(std::ranges::equal(transformed, expected));

    auto transformed2 = NonConstView(a, a + 4) | exe::scan(std::plus{}, 10);
    int  expected2[]  = {11, 13, 16, 20};
    ASSERT_TRUE(std::ranges::equal(transformed2, expected2));
}

TEST(ScanView, JoinArray) {
    int  a[]     = {1, 2, 3, 4};
    int  b[]     = {4, 3, 2, 1};
    auto out     = joinArrays(a, b);
    int  check[] = {1, 4, 6, 7};
    ASSERT_TRUE(std::ranges::equal(out, check));

    auto out2     = joinArrays(a, b, std::multiplies{});
    int  check2[] = {1, 3, 6, 6};
    ASSERT_TRUE(std::ranges::equal(out2, check2));
}

#if __cplusplus >= 202302L && __cpp_lib_ranges >= 202207L // FTM for P2494R2
TEST(ScanView, MoveOnly) {
    std::vector<std::unique_ptr<int>> vec4;
    vec4.push_back(std::make_unique<int>(5));
    vec4.push_back(std::make_unique<int>(2));
    vec4.push_back(std::make_unique<int>(10));
    auto out     = exe::scan(vec4, [](const auto& a, const auto& b) { return a + *b; }, 3);
    int  check[] = {8, 10, 20};
    ASSERT_TRUE(std::ranges::equal(out, check));
}
#endif
