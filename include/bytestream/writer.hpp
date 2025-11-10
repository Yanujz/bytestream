#ifndef BYTESTREAM_WRITER_HPP
#define BYTESTREAM_WRITER_HPP

#include <bytestream/config.hpp>
#include <bytestream/reader.hpp>
#include <cstring>

namespace bytestream {

class Writer {
private:
    std::byte*          data_;
    std::size_t         size_;
    mutable std::size_t pos_;

    void check(std::size_t n) const {
        if (pos_ + n > size_) {
            throw OverflowException("write past end");
        }
    }

public:
    constexpr Writer() noexcept
        : data_(nullptr), size_(0), pos_(0) {}

    constexpr Writer(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), pos_(0) {}

    constexpr Writer(bytestream::span<std::byte> s) noexcept
        : data_(s.data()), size_(s.size()), pos_(0) {}

    template<typename T>
    constexpr Writer(bytestream::span<T> s) noexcept
        : data_(reinterpret_cast<std::byte*>(s.data())),
          size_(s.size_bytes()),
          pos_(0) {}

    std::size_t size() const noexcept { return size_; }
    std::size_t position() const noexcept { return pos_; }
    std::size_t remaining() const noexcept { return size_ - pos_; }
    bool        empty() const noexcept { return size_ == 0; }
    bool        exhausted() const noexcept { return pos_ >= size_; }

    void seek(std::size_t p) const {
        if (p > size_) {
            throw std::out_of_range("seek past end");
        }
        pos_ = p;
    }

    void rewind() const noexcept { pos_ = 0; }

    void skip(std::size_t n) const {
        check(n);
        pos_ += n;
    }

    void align(std::size_t alignment, std::byte fill = std::byte{0}) const {
        assert(alignment > 0 && detail::has_single_bit(alignment));
        std::size_t aligned = (pos_ + alignment - 1) & ~(alignment - 1);
        std::size_t pad     = aligned - pos_;
        if (pad) {
            fill_bytes(fill, pad);
        }
    }

    bool is_aligned(std::size_t alignment) const noexcept {
        return (pos_ % alignment) == 0;
    }

    Writer subview(std::size_t offset, std::size_t length = static_cast<std::size_t>(-1)) const {
        if (offset > size_) {
            throw std::out_of_range("subview offset");
        }
        std::size_t len = (length == static_cast<std::size_t>(-1)) ? (size_ - offset) : length;
        if (offset + len > size_) {
            throw std::out_of_range("subview length");
        }
        return Writer(data_ + offset, len);
    }

    template<typename T>
    void write(T v) const {
        static_assert(detail::is_arithmetic<T>::value, "T must be arithmetic (not bool/char)");
        check(sizeof(T));
        std::memcpy(data_ + pos_, &v, sizeof(T));
        pos_ += sizeof(T);
    }

    template<typename T>
    void write_le(T v) const {
        if constexpr (sizeof(T) > 1) {
            if (!is_little_endian()) {
                v = byteswap(v);
            }
        }
        write(v);
    }

    template<typename T>
    void write_be(T v) const {
        if constexpr (sizeof(T) > 1) {
            if (is_little_endian()) {
                v = byteswap(v);
            }
        }
        write(v);
    }

    void write_bytes(bytestream::span<const std::byte> src) const {
        check(src.size());
        std::memcpy(data_ + pos_, src.data(), src.size());
        pos_ += src.size();
    }

    void write_bytes(const void* src, std::size_t n) const {
        check(n);
        std::memcpy(data_ + pos_, src, n);
        pos_ += n;
    }

    template<typename T>
    void write_array(bytestream::span<const T> src) const {
        static_assert(detail::is_arithmetic<T>::value, "T must be arithmetic");
        check(src.size_bytes());
        std::memcpy(data_ + pos_, src.data(), src.size_bytes());
        pos_ += src.size_bytes();
    }

    // allow non-const span<T>
    template<typename T>
    void write_array(bytestream::span<T> src) const {
        write_array(bytestream::span<const T>(src.data(), src.size()));
    }

    template<typename T>
    void write_array_le(bytestream::span<const T> src) const {
        if constexpr (is_little_endian() || sizeof(T) == 1) {
            write_array(src);
        } else {
            for (const auto &x : src) {
                write_le(x);
            }
        }
    }

    template<typename T>
    void write_array_le(bytestream::span<T> src) const {
        write_array_le(bytestream::span<const T>(src.data(), src.size()));
    }

    template<typename T>
    void write_array_be(bytestream::span<const T> src) const {
        if constexpr (is_big_endian() || sizeof(T) == 1) {
            write_array(src);
        } else {
            for (const auto &x : src) {
                write_be(x);
            }
        }
    }

    template<typename T>
    void write_array_be(bytestream::span<T> src) const {
        write_array_be(bytestream::span<const T>(src.data(), src.size()));
    }

    void write_string(std::string_view s) const {
        write_bytes(s.data(), s.size());
    }

    void write_sized_string_le(std::string_view s) const {
        write_le<std::uint32_t>(static_cast<std::uint32_t>(s.size()));
        write_string(s);
    }

    void write_sized_string_be(std::string_view s) const {
        write_be<std::uint32_t>(static_cast<std::uint32_t>(s.size()));
        write_string(s);
    }

    void write_cstring(std::string_view s) const {
        write_bytes(s.data(), s.size());
        write<std::uint8_t>(0);
    }

    void fill_bytes(std::byte b, std::size_t n) const {
        check(n);
        std::memset(data_ + pos_, static_cast<int>(b), n);
        pos_ += n;
    }

    void zero_fill(std::size_t n) const {
        fill_bytes(std::byte{0}, n);
    }

    Reader as_reader() const noexcept {
        return Reader(data_, size_);
    }
};

} // namespace bytestream

#endif // BYTESTREAM_WRITER_HPP
