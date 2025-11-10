# ByteStream

A modern, high-performance binary I/O library for C++17/20 that provides zero-copy, type-safe access to binary data with automatic endianness conversion.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Features

- 🚀 **Zero-Copy Design**: Views over existing buffers without allocation
- 🔒 **Type-Safe**: Strong type checking with SFINAE and C++20 concepts (when available)
- 🔄 **Endianness Support**: Automatic conversion between little/big/native endian
- 📦 **Header-Only**: Easy integration, no build required
- 🎯 **Modern C++**: C++17 compatible, enhanced with C++20 features when available
- ⚡ **High Performance**: Compiler intrinsics for byte swapping
- 🛡️ **Exception Safety**: Comprehensive bounds checking
- 📐 **Alignment Support**: Built-in alignment helpers
- 🧪 **Well-Tested**: Extensive unit test coverage with GoogleTest

## Quick Start

### Installation

```bash
git clone https://github.com/yourusername/bytestream.git
cd bytestream
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```

### Basic Usage

```cpp
#include <bytestream/core.hpp>
#include <vector>

using namespace bytestream;

int main() {
    std::vector<uint8_t> buffer(256);
    
    // Writing data
    {
        Writer writer(buffer.data(), buffer.size());
        writer.write_le<uint32_t>(0xDEADBEEF);
        writer.write_be<uint16_t>(0x1234);
        writer.write<float>(3.14159f);
        writer.write_sized_string_le("Hello, World!");
    }
    
    // Reading data
    {
        Reader reader(buffer.data(), buffer.size());
        auto value1 = reader.read_le<uint32_t>();
        auto value2 = reader.read_be<uint16_t>();
        auto value3 = reader.read<float>();
        auto text = reader.read_sized_string_le();
    }
    
    return 0;
}
```

## Core Classes

### Reader

Immutable, read-only view over binary data.

```cpp
Reader reader(data, size);

// Read primitives
uint32_t val = reader.read_le<uint32_t>();  // Little-endian
uint16_t val = reader.read_be<uint16_t>();  // Big-endian
float val = reader.read<float>();            // Native endian

// Peek without advancing position
uint8_t next = reader.peek<uint8_t>();

// Strings
std::string str = reader.read_string(10);
std::string str = reader.read_sized_string_le();
std::string str = reader.read_cstring();
std::string_view view = reader.view_string(10);  // Zero-copy

// Arrays
std::array<int32_t, 10> data;
reader.read_array_le(std::span(data));

// Position control
reader.seek(100);
reader.rewind();
reader.skip(20);
reader.align(16);

// Subviews
Reader sub = reader.subview(offset, length);
```

### Writer

Mutable view over binary data for writing.

```cpp
Writer writer(data, size);

// Write primitives
writer.write_le<uint32_t>(0x12345678);  // Little-endian
writer.write_be<uint16_t>(0xABCD);      // Big-endian
writer.write<double>(3.14159);           // Native endian

// Strings
writer.write_string("Hello");
writer.write_sized_string_le("Hello");
writer.write_cstring("Hello");

// Arrays
std::array<float, 5> data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
writer.write_array_be(std::span(data));

// Fill operations
writer.zero_fill(100);
writer.fill_bytes(std::byte{0xFF}, 50);

// Alignment
writer.align(16, std::byte{0x00});

// Convert to Reader
Reader reader = writer.as_reader();
```

### Stream

Bidirectional view combining Reader and Writer functionality.

```cpp
Stream stream(data, size);
Reader reader = stream.reader();
Writer writer = stream.writer();
```

## Endianness Support

ByteStream provides explicit control over byte order:

```cpp
// Little-endian (least significant byte first)
writer.write_le<uint32_t>(0x12345678);  // Stores: 78 56 34 12

// Big-endian (most significant byte first)
writer.write_be<uint32_t>(0x12345678);  // Stores: 12 34 56 78

// Native endian (matches system)
writer.write<uint32_t>(0x12345678);     // Depends on platform
```

Automatic detection and conversion:

```cpp
if (is_little_endian()) {
    // System is little-endian
}

// Manual byte swapping
uint32_t swapped = byteswap(0x12345678);
```

## Advanced Features

### Alignment

```cpp
Writer writer(buffer.data(), buffer.size());

writer.write<uint8_t>(0x42);
writer.align(16, std::byte{0x00});  // Pad to 16-byte boundary

if (writer.is_aligned(16)) {
    // Now aligned
}
```

### Subviews

```cpp
Reader reader(buffer.data(), buffer.size());

// Create view of subset
Reader sub = reader.subview(100, 50);  // Offset 100, length 50
Reader sub = reader.subview(100);       // From offset 100 to end
```

### Error Handling

```cpp
try {
    Reader reader(buffer.data(), buffer.size());
    reader.read<uint64_t>();  // May throw if insufficient data
}
catch (const UnderflowException& e) {
    // Handle read past end
}
catch (const OverflowException& e) {
    // Handle write past end
}
```

## Building

### Requirements

- C++17 compliant compiler (GCC 7+, Clang 5+, MSVC 2017+)
- C++20 recommended for enhanced features (std::span, std::endian, concepts)
- CMake 3.20+
- GoogleTest (automatically fetched for tests)

### Build Options

```bash
cmake -B build \
  -DBYTESTREAM_BUILD_TESTS=ON \
  -DBYTESTREAM_BUILD_EXAMPLES=ON \
  -DBYTESTREAM_BUILD_BENCHMARKS=OFF \
  -DBYTESTREAM_ENABLE_SANITIZERS=OFF \
  -DBYTESTREAM_ENABLE_COVERAGE=OFF

cmake --build build
```

### Running Tests

```bash
cd build
ctest --output-on-failure
# or
./tests/bytestream_tests
```

### Integration with CMake

Using `find_package`:

```cmake
find_package(ByteStream REQUIRED)
target_link_libraries(your_target PRIVATE ByteStream::bytestream)
```

Using `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(
    bytestream
    GIT_REPOSITORY https://github.com/yourusername/bytestream.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(bytestream)

target_link_libraries(your_target PRIVATE ByteStream::bytestream)
```

As a subdirectory:

```cmake
add_subdirectory(external/bytestream)
target_link_libraries(your_target PRIVATE ByteStream::bytestream)
```

## Performance

ByteStream is designed for maximum performance:

- **Zero-copy operations**: No unnecessary memory allocations
- **Compiler intrinsics**: Uses `__builtin_bswap*` / `_byteswap_*` for byte swapping
- **Inline functions**: Most operations inline to eliminate overhead
- **Modern CPU features**: Leverages C++20 optimizations
- **Cache-friendly**: Sequential access patterns

## Use Cases

- **Network protocols**: Parse/serialize binary network packets
- **File formats**: Read/write custom binary file formats
- **Serialization**: High-performance data serialization
- **Game development**: Asset loading, save files, network replication
- **Embedded systems**: Efficient binary communication
- **Audio/Video processing**: Frame parsing and encoding

## Examples

See the [examples](examples/) directory for complete examples:

- `basic_usage.cpp` - Basic reading and writing
- Protocol parsing
- Binary file formats
- Network packet handling

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by Java's ByteBuffer and Rust's std::io
- Uses compiler intrinsics from GCC, Clang, and MSVC
- Built with modern C++20 features

## Version History

### 1.0.0 (2025-01-10)
- Initial release
- Reader, Writer, and Stream classes
- Full endianness support
- Comprehensive test suite
- Examples and documentation