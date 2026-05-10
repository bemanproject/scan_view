# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_SCAN_VIEW_USE_MODULES)
    target_sources(
        beman.scan_view
        PUBLIC
            FILE_SET CXX_MODULES FILES scan_view.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    scan_view.hpp
                    scan.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/scan_view/config_generated.hpp"
    )
else()
    target_sources(
        beman.scan_view
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    scan_view.hpp
                    scan.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/scan_view/config_generated.hpp"
    )
endif()