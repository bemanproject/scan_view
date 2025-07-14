#include <functional>
#include <iostream>
#include <vector>

#if __cpp_lib_print >= 202207L
#include <print>
#endif

#include <beman/scan_view/scan.hpp>

namespace exe = beman::scan_view;

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

// Example given in the paper for `views::scan`. (Needs C++23)
int main() {
    std::vector vec{1, 2, 3, 4, 5, 4, 3, 2, 1};

    // [1, 3, 6, 10, 15, 19, 22, 24, 25]
    println(vec | exe::scan(std::plus{}));
    // [11, 13, 16, 20, 25, 29, 32, 34, 35]
    println(vec | exe::scan(std::plus{}, 10));
    // [1, 2, 3, 4, 5, 5, 5, 5, 5]
    println(vec | exe::scan(std::ranges::max));
    // [3, 3, 3, 4, 5, 5, 5, 5, 5]
    println(vec | exe::scan(std::ranges::max, 3));

    return 0;
}
