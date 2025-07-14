#include <ranges>
#include <vector>
#include <print>
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <memory>

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

int main() {
    std::vector vec{1, 2, 3, 4, 5, 4, 3, 2, 1};
    std::print("{}\n", exe::partial_sum(vec));
    const auto R = exe::scan(vec, std::ranges::max);
    std::println("{}", R);
    std::print("{}\n", exe::prescan(std::as_const(vec), std::plus{}, 10));

    std::vector vec2{1, 2147483647, 20, 3};
    std::print("{}\n", exe::prescan(vec2, std::plus{}, 0LL));
    static_assert(
        std::is_same_v<std::decay_t<ranges::range_reference_t<decltype(exe::prescan(vec2, std::plus{}, 0LL))>>,
                       long long>);

    std::vector vec3{1, 2, 3};
    std::print(
        "{}\n",
        exe::prescan(
            vec3, [](const auto& a, const auto& b) mutable { return std::to_string(a) + std::to_string(b); }, "2"));

    std::vector<std::unique_ptr<int>> vec4;
    vec4.push_back(std::make_unique<int>(5));
    vec4.push_back(std::make_unique<int>(2));
    vec4.push_back(std::make_unique<int>(10));
    std::println("{}", exe::prescan(vec4, [](const auto& a, const auto& b) { return a + *b; }, 3));
    std::println("{}",
                 exe::prescan(
                     vec3, [](const auto& a, const auto& b) { return std::make_unique<int>(*a + b); }, A{}) |
                     views::transform([](const auto& a) { return *a; }));
}
