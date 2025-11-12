# ByteStream

[![Build](https://img.shields.io/github/actions/workflow/status/yanujz/bytestream/ci.yml?label=build&branch=main)](https://github.com/yanujz/bytestream/actions)
[![Docs](https://img.shields.io/website?url=https%3A%2F%2Fyanujz.github.io%2Fbytestream&label=docs)](https://yanujz.github.io/bytestream)
[![Coverage](https://img.shields.io/endpoint?url=https://yanujz.github.io/bytestream/badges/coverage.json)](https://yanujz.github.io/bytestream/coverage/)

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Build System](https://img.shields.io/badge/build-CMake-blueviolet)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
![Platforms](https://img.shields.io/badge/platforms-linux%20%7C%20macOS%20%7C%20windows-informational)

[![Release](https://img.shields.io/github/v/release/yanujz/bytestream)](https://github.com/yanujz/bytestream/releases)
[![Last commit](https://img.shields.io/github/last-commit/yanujz/bytestream?logo=github)](https://github.com/yanujz/bytestream/commits)
[![Issues](https://img.shields.io/github/issues/yanujz/bytestream)](https://github.com/yanujz/bytestream/issues)
[![PRs](https://img.shields.io/github/issues-pr/yanujz/bytestream)](https://github.com/yanujz/bytestream/pulls)


A simple, modern C++17/20 library for binary I/O on fixed-size memory buffers.

---

## Features

- **Header-only:** just add `include/` to your project.
- **Modern C++:** `std::byte`, `std::string_view`, `std::span` (with a C++17 fallback in the library).
- **Clear API:** separate `bytestream::Reader` (read-only) and `bytestream::Writer` (mutable).
- **Explicit endianness:** `read_le` / `read_be` / `write_le` / `write_be`.
- **Safe:** throws `bytestream::UnderflowException` / `OverflowException` on out-of-bounds.
- **Navigation helpers:** `seek()`, `skip()`, `position()`, `align()`.
- **CMake target:** `ByteStream::bytestream`.

---

## Requirements

- CMake **3.14+**
- C++ **17+** compiler (GCC, Clang, MSVC)
- Optional: Git (to embed a short commit hash in the generated `version.hpp`)

---

## Usage

Because it’s header-only, you can just copy `include/bytestream/` into your project and:

```cpp
#include <bytestream/core.hpp>
````

### Using CMake (FetchContent)

```cmake
include(FetchContent)

FetchContent_Declare(
  bytestream
  GIT_REPOSITORY https://github.com/yanujz/bytestream.git
  GIT_TAG        main  # or a release tag
)

FetchContent_MakeAvailable(bytestream)

target_link_libraries(your_target PRIVATE ByteStream::bytestream)
```

### Using CMake (installed)

```cmake
find_package(ByteStream 0.1.0 REQUIRED)
target_link_libraries(your_target PRIVATE ByteStream::bytestream)
```

---

## Example

```cpp
#include <bytestream/stream.hpp>
#include <iostream>
#include <array>
#include <cassert>

int main() {
    std::array<std::byte, 64> buffer{};

    bytestream::Stream stream(buffer.data(), buffer.size());

    try {
        bytestream::Writer writer = stream.writer();
        writer.write_le<std::uint32_t>(0xDEADBEEF);
        writer.write_be<std::uint16_t>(12345);
        writer.write_cstring("Hello, bytestream!");

        std::cout << "Wrote " << writer.position() << " bytes.\n";

        bytestream::Reader reader = stream.reader();

        std::uint32_t v1 = reader.read_le<std::uint32_t>();
        std::uint16_t v2 = reader.read_be<std::uint16_t>();
        std::string   v3 = reader.read_cstring();

        std::cout << "Read value 1: 0x" << std::hex << v1 << "\n";
        std::cout << "Read value 2: " << std::dec << v2 << "\n";
        std::cout << "Read value 3: \"" << v3 << "\"\n";

        assert(v1 == 0xDEADBEEF);
        assert(v2 == 12345);
        assert(v3 == "Hello, bytestream!");
    } catch (const bytestream::Exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
```

## Building the project

```bash
git clone https://github.com/yanujz/bytestream.git
cd bytestream
cmake -S . -B build \
  -DBYTESTREAM_BUILD_TESTS=ON \
  -DBYTESTREAM_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## CMake options

| Option                         | Description                    | Default |
| ------------------------------ | ------------------------------ | ------- |
| `BYTESTREAM_BUILD_TESTS`       | Build the unit tests           | `ON`    |
| `BYTESTREAM_BUILD_EXAMPLES`    | Build the examples             | `ON`    |
| `BYTESTREAM_BUILD_BENCHMARKS`  | Build benchmarks (placeholder) | `OFF`   |
| `BYTESTREAM_INSTALL`           | Install headers/targets        | `ON`    |
| `BYTESTREAM_ENABLE_SANITIZERS` | Enable ASan/UBSan if supported | `OFF`   |
| `BYTESTREAM_ENABLE_COVERAGE`   | Enable coverage flags          | `OFF`   |

---

## Generated version header

Configure step generates:

* from: `include/bytestream/version.hpp.in`
* to:   `build/include/bytestream/version.hpp`

It defines macros and C++ constants like:

* `BYTESTREAM_VERSION_MAJOR`, `BYTESTREAM_VERSION_MINOR`, `BYTESTREAM_VERSION_PATCH`
* `bytestream::version_string`
* `bytestream::complete_name`
* `bytestream::info()`

Include it with:

```cpp
#include <bytestream/version.hpp>
```

## Docs
* Site: [https://yanujz.github.io/bytestream](https://yanujz.github.io/bytestream)
* Changelog: [https://yanujz.github.io/bytestream/changelog/](https://yanujz.github.io/bytestream/changelog/)


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
