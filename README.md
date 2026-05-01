# beman.scan_view: A range adaptor that is a lazy view version of `std::inclusive_scan`

<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

<!-- markdownlint-disable-next-line line-length -->
![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg) ![Continuous Integration Tests](https://github.com/bemanproject/scan_view/actions/workflows/ci_tests.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/bemanproject/scan_view/actions/workflows/pre-commit-check.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/bemanproject/scan_view/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/scan_view?branch=main) ![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg) [![Compiler Explorer Example](https://img.shields.io/badge/Try%20it%20on%20Compiler%20Explorer-grey?logo=compilerexplorer&logoColor=67c52a)](https://godbolt.org/z/qMvEhWKf9)

**Implements**: `std::views::scan` proposed in [`views::scan` (P3351R4)](https://wg21.link/P3351R4).

**Difference from the paper**:

- Implemented `reserve_hint` are hidden behind a feature-test macro.

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

## License

`beman.scan_view` is licensed under the Apache License v2.0 with LLVM Exceptions.

## Usage

`views::scan` is a range adaptor that takes a range and a function that
takes the current element and the current state as parameters.
Basically, `views::scan` is a lazy view version of `std::inclusive_scan`, or `views::transform` with a stateful function.

```cpp
#include <functional>
#include <print>
#include <vector>

#include <beman/scan_view/scan.hpp>

namespace exe = beman::scan_view;

// Example given in the paper for `views::scan`. (Needs C++23)
int main()
{
    std::vector vec{1, 2, 3, 4, 5, 4, 3, 2, 1};

    // [1, 3, 6, 10, 15, 19, 22, 24, 25]
    std::println("{}", vec | exe::scan(std::plus{}));
    // [11, 13, 16, 20, 25, 29, 32, 34, 35]
    std::println("{}", vec | exe::scan(std::plus{}, 10));
    // [1, 2, 3, 4, 5, 5, 5, 5, 5]
    std::println("{}", vec | exe::scan(std::ranges::max));
    // [3, 3, 3, 4, 5, 5, 5, 5, 5]
    std::println("{}", vec | exe::scan(std::ranges::max, 3));

    return 0;
}
```

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_SCAN_VIEW_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 15-13   | C++26-C++20   | libstdc++         |
| GCC        | 12-11   | C++23, C++20  | libstdc++         |
| Clang      | 20-19   | C++26-C++20   | libstdc++, libc++ |
| Clang      | 18-17   | C++26-C++20   | libc++            |
| Clang      | 18-17   | C++20         | libstdc++         |
| AppleClang | latest  | C++26-C++20   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.scan_view into your project

### Build

You can build scan_view using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.scan_view without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install scan_view is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which scan_view [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-scan-view`.

#### Manual

To install beman.scan_view globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── scan_view
│           ├── scan.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.scan_view
            ├── beman.scan_view-config-version.cmake
            ├── beman.scan_view-config.cmake
            └── beman.scan_view-targets.cmake
```

### CMake Configuration

If you installed beman.scan_view to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.scan_view` package to define the `beman::scan_view` CMake
target:

```cmake
find_package(beman.scan_view REQUIRED)
```

You will then need to add `beman::scan_view` to the link libraries of any libraries or
executables that include `beman.scan_view` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::scan_view)
```

### Using beman.scan_view

To use `beman.scan_view` in your C++ project,
include an appropriate `beman.scan_view` header from your source code.

```c++
#include <beman/scan_view/scan.hpp>
```

> [!NOTE]
>
> `beman.scan_view` headers are to be included with the `beman/scan_view/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <scan.hpp>`) is unsupported.
