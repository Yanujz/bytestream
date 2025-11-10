# ByteStream

Small, header-only C++17/20 library for reading and writing binary data on top of existing buffers.

- ✅ Header-only, CMake target: `ByteStream::bytestream`
- ✅ Reader/Writer/Stream views
- ✅ Endianness helpers
- ✅ Alignment + subviews
- ✅ Tested with GoogleTest

Use this for:
- parsing binary protocols
- serializing small packets
- working with fixed buffers (network, embedded, game tooling)

```cmake
find_package(ByteStream REQUIRED)
target_link_libraries(app PRIVATE ByteStream::bytestream)
```
