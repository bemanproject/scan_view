# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.scan_view.tests.identity)
target_sources(beman.scan_view.tests.identity PRIVATE identity.test.cpp)
target_link_libraries(
    beman.scan_view.tests.identity
    PRIVATE beman::scan_view GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beman.scan_view.tests.identity)
