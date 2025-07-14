# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES basic_example complex_example)

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.scan_view.examples.${example})
    target_sources(beman.scan_view.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(
        beman.scan_view.examples.${example}
        PRIVATE beman::scan_view
    )
endforeach()
