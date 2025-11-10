# Core API

## Namespaces

- `bytestream` – main namespace
- `bytestream::detail` – internal helpers (not stable)

## Exceptions

- `bytestream::Exception`
- `bytestream::OverflowException`
- `bytestream::UnderflowException`
- `bytestream::AlignmentException`
- `bytestream::AccessException`

All derive from `std::runtime_error`.

## Reader

```cpp
bytestream::Reader r(const void* data, std::size_t size);

auto u32  = r.read_le<std::uint32_t>();
auto str  = r.read_cstring();

r.seek(10);
r.align(4);
auto sub = r.subview(20, 16);
````

Key ops:

* `read<T>()` – native-endian
* `read_le<T>()` / `read_be<T>()`
* `read_bytes(...)`
* `read_string(len)` / `read_sized_string_le()` / `read_cstring()`
* `seek()`, `rewind()`, `skip()`, `align()`
* `subview(offset, length)`

## Writer

```cpp
bytestream::Writer w(void* data, std::size_t size);

w.write_le<std::uint16_t>(0x1234);
w.write_cstring("hi");
w.align(4);
```

Key ops:

* `write<T>()`
* `write_le<T>()` / `write_be<T>()`
* `write_bytes(...)`
* `write_string(...)`, `write_sized_string_le(...)`, `write_cstring(...)`
* `align(alignment, fill_byte)`

## Endianness helpers

```cpp
bool bytestream::is_little_endian();
bool bytestream::is_big_endian();

auto swapped = bytestream::byteswap(std::uint32_t{0x12345678});
```

## Version

```cpp
#include <bytestream/version.hpp>

std::cout << bytestream::version_string << "\n";
auto info = bytestream::info();
```