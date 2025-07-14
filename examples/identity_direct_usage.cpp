// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scan_view/identity.hpp>

#include <iostream>

namespace exe = beman::scan_view;

int main() {
    std::cout << exe::identity()(2024) << '\n';
    return 0;
}
