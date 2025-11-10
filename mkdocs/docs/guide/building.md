# Building

ByteStream is header-only, but the repo ships with tests and examples.

## Prerequisites

- CMake 3.14+
- C++17 compiler (GCC, Clang, MSVC)
- Git (optional, used to embed the current commit into `version.hpp`)

## Configure

```bash
cmake -S . -B build \
  -DBYTESTREAM_BUILD_TESTS=ON \
  -DBYTESTREAM_BUILD_EXAMPLES=ON
````

## Build

```bash
cmake --build build
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Install (optional)

```bash
cmake --install build
```

This installs headers and the CMake package so you can do:

```cmake
find_package(ByteStream REQUIRED)
target_link_libraries(your_app PRIVATE ByteStream::bytestream)
```

````

---

### `docs/guide/quick_start.md`

```markdown
# Quick Start

## 1. Add as subdirectory

```cmake
add_subdirectory(external/bytestream)
target_link_libraries(my_app PRIVATE ByteStream::bytestream)
````

## 2. Write some data

```cpp
#include <bytestream/core.hpp>
#include <vector>

int main() {
    std::vector<std::uint8_t> buf(128);

    bytestream::Writer w(buf.data(), buf.size());
    w.write_le<std::uint32_t>(0xDEADBEEF);
    w.write_cstring("hello");

    bytestream::Reader r(buf.data(), buf.size());
    auto v  = r.read_le<std::uint32_t>();
    auto s  = r.read_cstring();

    (void)v; (void)s;
}
```

That’s it. No linking step, it’s header-only.