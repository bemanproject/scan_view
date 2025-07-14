#include <ranges>
#include <vector>
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <memory>
#include <iostream>

#if __cpp_lib_print >= 202207L
#include <print>
#endif

#include <beman/scan_view/scan.hpp>

namespace std {
string to_string(string_view str) { return string{str}; }
} // namespace std

namespace ranges = std::ranges;
namespace views  = std::views;
namespace exe    = beman::scan_view;

struct A {
    operator std::unique_ptr<int>() const { return std::make_unique<int>(); }
    int operator*() const { return 0; }
};

#if __cpp_lib_print >= 202207L && __cpp_lib_format_ranges >= 202207L
void print(auto&& rng) { std::print("{}", std::forward<decltype(rng)>(rng)); }

void println(auto&& rng) { std::println("{}", std::forward<decltype(rng)>(rng)); }
#else
void print(auto&& rng) {
    std::cout << "[";
    bool first = true;
    for (auto&& elem : rng) {
        if (first)
            first = false;
        else
            std::cout << ", ";
        std::cout << elem;
    }
    std::cout << "]";
}

void println(auto&& rng) {
    print(std::forward<decltype(rng)>(rng));
    std::cout << "\n";
}
#endif

int main() {
    std::vector vec{1, 2, 3, 4, 5, 4, 3, 2, 1};
    println(exe::partial_sum(vec));
    const auto R = exe::scan(vec, std::ranges::max);
    println(R);
    println(exe::prescan(std::as_const(vec), std::plus{}, 10));

    std::vector vec2{1, 2147483647, 20, 3};
    println(exe::prescan(vec2, std::plus{}, 0LL));
    static_assert(
        std::is_same_v<std::decay_t<ranges::range_reference_t<decltype(exe::prescan(vec2, std::plus{}, 0LL))>>,
                       long long>);

    std::vector vec3{1, 2, 3};
    println(exe::prescan(
        vec3, [](const auto& a, const auto& b) mutable { return std::to_string(a) + std::to_string(b); }, "2"));

#if __cplusplus >= 202302L && __cpp_lib_ranges >= 202207L // FTM for P2494R2
    std::vector<std::unique_ptr<int>> vec4;
    vec4.push_back(std::make_unique<int>(5));
    vec4.push_back(std::make_unique<int>(2));
    vec4.push_back(std::make_unique<int>(10));
    println(exe::prescan(vec4, [](const auto& a, const auto& b) { return a + *b; }, 3));
    println(exe::prescan(
                vec3, [](const auto& a, const auto& b) { return std::make_unique<int>(*a + b); }, A{}) |
            views::transform([](const auto& a) { return *a; }));
#endif
}
