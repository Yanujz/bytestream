#ifndef BYTESTREAM_WRITER_HPP
#define BYTESTREAM_WRITER_HPP

#include <bytestream/config.hpp>
#include <cstring>
#include <string>
#include <string_view>

namespace bytestream {

class Reader; // fwd

class Writer {
    std::byte*  data_;
    std::size_t size_;
    std::size_t pos_;
public:
    Writer(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), pos_(0) {}

    std::size_t size() const noexcept { return size_; }
    std::size_t position() const noexcept { return pos_; }
    std::size_t remaining() const noexcept { return size_ - pos_; }
    // Convenience alias for readability
    std::size_t written_bytes() const noexcept { return pos_; }

    void seek(std::size_t p) {
        if (p > size_) throw std::out_of_range("bytestream::Writer seek past end");
        pos_ = p;
    }

    void ensure(std::size_t n) const {
        if (n > (size_ - pos_)) throw OverflowException("bytestream::Writer overflow");
    }

    // ---- raw bytes
    void write_bytes(const void* src, std::size_t n) {
        ensure(n);
        std::memcpy(data_ + pos_, src, n);
        pos_ += n;
    }

    // ---- trivially-copyable write
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable<T>::value, void>
    write(const T& v) { write_bytes(&v, sizeof(T)); }

    // ---- arithmetic endian-aware
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, void>
    write_le(T v) {
        if constexpr (is_big_endian()) v = byteswap(v);
        write(v);
    }
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, void>
    write_be(T v) {
        if constexpr (is_little_endian()) v = byteswap(v);
        write(v);
    }

    // ---- arrays
    template <typename T>
    void write_array(span<const T> arr) {
        write_bytes(arr.data(), arr.size() * sizeof(T));
    }
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, void>
    write_array_be(span<const T> arr) {
        for (std::size_t i = 0; i < arr.size(); ++i) write_be<T>(arr.data()[i]);
    }

    // ---- strings
    void write_string(std::string_view s) { write_bytes(s.data(), s.size()); }

    void write_sized_string_le(std::string_view s) {
        write_le<std::uint32_t>(static_cast<std::uint32_t>(s.size()));
        write_string(s);
    }
    void write_sized_string_be(std::string_view s) {
        write_be<std::uint32_t>(static_cast<std::uint32_t>(s.size()));
        write_string(s);
    }
    void write_cstring(std::string_view s) {
        write_string(s);
        const std::uint8_t zero = 0;
        write(zero);
    }

    // ---- fills & alignment
    void fill_bytes(std::byte value, std::size_t count) {
        ensure(count);
        std::memset(data_ + pos_, int(value), count);
        pos_ += count;
    }
    void zero_fill(std::size_t count) { fill_bytes(std::byte{0}, count); }

    void align(std::size_t alignment, std::byte fill = std::byte{0}) {
        std::size_t next = align_up(pos_, alignment);
        std::size_t pad  = next - pos_;
        if (pad) fill_bytes(fill, pad);
    }

    // ---- native helpers used by serialization.hpp
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable<T>::value, void>
    write_native(const T& v) { write(v); }

    Reader as_reader() const noexcept;
};

} // namespace bytestream

// Provide definition once Reader is visible
#include <bytestream/reader.hpp>

namespace bytestream {
inline Reader Writer::as_reader() const noexcept {
    return Reader{data_, size_};
}
}

#endif // BYTESTREAM_WRITER_HPP
