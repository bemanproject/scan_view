# Examples for beman.scan_view

<!--
SPDX-License-Identifier: 2.0 license with LLVM exceptions
-->

List of usage examples for `beman.scan_view`.

## Samples

Check basic `beman.scan_view` library usages:

* local [./basic_example.cpp](./basic_example.cpp) or [basic_example@Compiler Explorer](https://godbolt.org/z/sjYcq61oz)
* local [./complex_example.cpp](./complex_example.cpp) or [complex_example@Compiler Explorer](https://godbolt.org/z/9qfd643bE)

### Local Build and Run

```bash
# building
$ cmake --workflow --preset llvm-release

# run sample.cpp
$ ./build/llvm-release/examples/beman.scan_view.examples.basic_example
[1, 3, 6, 10, 15, 19, 22, 24, 25]
[11, 13, 16, 20, 25, 29, 32, 34, 35]
[1, 2, 3, 4, 5, 5, 5, 5, 5]
[3, 3, 3, 4, 5, 5, 5, 5, 5]
```
