# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

find_package(GTest REQUIRED)

add_executable(beman.scan_view.tests.basic)
target_sources(beman.scan_view.tests.basic PRIVATE basic.test.cpp)
target_link_libraries(
    beman.scan_view.tests.basic
    PRIVATE beman::scan_view GTest::gtest GTest::gtest_main
)

if(BEMAN_SCAN_VIEW_USE_MODULES)
    set_target_properties(
        beman.scan_view.tests.basic
        PROPERTIES CXX_MODULE_STD ON
    )
endif()

include(GoogleTest)
gtest_discover_tests(beman.scan_view.tests.basic DISCOVERY_TIMEOUT 60)
