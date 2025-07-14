# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    beman.scan_view # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "A range adaptor that is a lazy view version of `std::inclusive_scan`"
    LANGUAGES CXX
    VERSION 2.1.1
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_SCAN_VIEW_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_SCAN_VIEW_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    BEMAN_SCAN_VIEW_INSTALL_CONFIG_FILE_PACKAGE
    "Enable creating and installing a CMake config-file package. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

include(CTest)

add_subdirectory(src/beman/scan_view)

if(BEMAN_SCAN_VIEW_BUILD_TESTS)
    add_subdirectory(tests/beman/scan_view)
endif()

if(BEMAN_SCAN_VIEW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
