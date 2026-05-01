# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

project(
    beman.scan_view
    DESCRIPTION
        "A range adaptor that is a lazy view version of std::inclusive_scan"
    LANGUAGES CXX
    VERSION 0.1.0
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

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

add_library(beman.scan_view INTERFACE)
add_library(beman::scan_view ALIAS beman.scan_view)

target_sources(
    beman.scan_view
    PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(
    beman.scan_view
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(include/beman/scan_view)

beman_install_library(beman.scan_view TARGETS beman.scan_view)
configure_build_telemetry()

if(BEMAN_SCAN_VIEW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/scan_view)
endif()

if(BEMAN_SCAN_VIEW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
