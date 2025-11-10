#ifndef BYTESTREAM_WRITER_HPP
#define BYTESTREAM_WRITER_HPP

#include <bytestream/config.hpp>
#include <bytestream/reader.hpp>
#include <limits>
#include <cstring>

namespace bytestream {

class Writer {
private:
    std::byte*         data_;
    std::size_t        size_;
    mutable std::size_t position_;

    void check_bounds(std::size_t bytes) const {
        if (position_ + bytes > size_) {
            throw OverflowException(
                "Attempted to write " + std::to_string(bytes) +
                " bytes at position " + std::to_string(position_) +
                " (size: " + std::to_string(size_) + ")"
            );
        }
    }

public:
    constexpr Writer() noexcept
        : data_(nullptr), size_(0), position_(0) {}

    constexpr Writer(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), position_(0) {}

    constexpr Writer(bytestream::span<std::byte> s) noexcept
        : data_(s.data()), size_(s.size()), position_(0) {}

    template<typename T>
    constexpr Writer(bytestream::span<T> s) noexcept
        : data_(reinterpret_cast<std::byte*>(s.data())),
          size_(s.size_bytes()), position_(0) {}

    // ---------------------------------------------------------------------
    // props
    // ---------------------------------------------------------------------
    [[nodiscard]] constexpr std::byte* data() const noexcept { return data_; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
    [[nodiscard]] constexpr std::size_t position() const noexcept { return position_; }
    [[nodiscard]] constexpr std::size_t remaining() const noexcept { return size_ - position_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] constexpr bool exhausted() const noexcept { return position_ >= size_; }

    // ---------------------------------------------------------------------
    // position
    // ---------------------------------------------------------------------
    void seek(std::size_t pos) const {
        if (pos > size_) {
            throw std::out_of_range("Seek position exceeds size");
        }
        position_ = pos;
    }

    constexpr void rewind() const noexcept { position_ = 0; }

    void skip(std::size_t bytes) const {
        check_bounds(bytes);
        position_ += bytes;
    }

    void align(std::size_t alignment, std::byte fill = std::byte{0}) const {
        assert(alignment > 0 && detail::has_single_bit(alignment));
        std::size_t aligned = (position_ + alignment - 1) & ~(alignment - 1);
        std::size_t pad     = aligned - position_;
        if (pad > 0) {
            fill_bytes(fill, pad);
        }
    }

    [[nodiscard]] bool is_aligned(std::size_t alignment) const noexcept {
        return (position_ % alignment) == 0;
    }

    // ---------------------------------------------------------------------
    // subview
    // ---------------------------------------------------------------------
    [[nodiscard]] Writer subview(std::size_t offset,
                                 std::size_t length = static_cast<std::size_t>(-1)) const
    {
        if (offset > size_) {
            throw std::out_of_range("Subview offset exceeds buffer size");
        }
        std::size_t actual = (length == static_cast<std::size_t>(-1))
                                 ? (size_ - offset)
                                 : length;
        if (offset + actual > size_) {
            throw std::out_of_range("Subview extends beyond buffer");
        }
        return Writer(data_ + offset, actual);
    }

    // ---------------------------------------------------------------------
    // arithmetic writes
    // ---------------------------------------------------------------------
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write(T value) const {
        check_bounds(sizeof(T));
        std::memcpy(data_ + position_, &value, sizeof(T));
        position_ += sizeof(T);
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_le(T value) const {
        if constexpr (sizeof(T) > 1) {
            if constexpr (is_big_endian()) {
                value = byteswap(value);
            }
        }
        write(value);
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_be(T value) const {
        if constexpr (sizeof(T) > 1) {
            if constexpr (is_little_endian()) {
                value = byteswap(value);
            }
        }
        write(value);
    }

    // ---------------------------------------------------------------------
    // bulk
    // ---------------------------------------------------------------------
    void write_bytes(bytestream::span<const std::byte> src) const {
        check_bounds(src.size());
        std::memcpy(data_ + position_, src.data(), src.size());
        position_ += src.size();
    }

    void write_bytes(const void* src, std::size_t count) const {
        check_bounds(count);
        std::memcpy(data_ + position_, src, count);
        position_ += count;
    }

    // main const-source array write
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array(bytestream::span<const T> src) const {
        check_bounds(src.size_bytes());
        std::memcpy(data_ + position_, src.data(), src.size_bytes());
        position_ += src.size_bytes();
    }

    // forwarding overload for non-const span<T>
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array(bytestream::span<T> src) const {
        write_array(bytestream::span<const T>(src.data(), src.size()));
    }

    // LE array
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array_le(bytestream::span<const T> src) const {
        if constexpr (is_little_endian() || sizeof(T) == 1) {
            write_array(src);
        } else {
            for (const auto &e : src) {
                write_le(e);
            }
        }
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array_le(bytestream::span<T> src) const {
        write_array_le(bytestream::span<const T>(src.data(), src.size()));
    }

    // BE array
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array_be(bytestream::span<const T> src) const {
        if constexpr (is_big_endian() || sizeof(T) == 1) {
            write_array(src);
        } else {
            for (const auto &e : src) {
                write_be(e);
            }
        }
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void write_array_be(bytestream::span<T> src) const {
        write_array_be(bytestream::span<const T>(src.data(), src.size()));
    }

    // ---------------------------------------------------------------------
    // strings
    // ---------------------------------------------------------------------
    void write_string(std::string_view s) const {
        write_bytes(s.data(), s.size());
    }

    void write_sized_string_le(std::string_view s) const {
        write_le<uint32_t>(static_cast<uint32_t>(s.size()));
        write_string(s);
    }

    void write_sized_string_be(std::string_view s) const {
        write_be<uint32_t>(static_cast<uint32_t>(s.size()));
        write_string(s);
    }

    void write_cstring(std::string_view s) const {
        write_bytes(s.data(), s.size());
        write<uint8_t>(0);
    }

    // ---------------------------------------------------------------------
    // fill
    // ---------------------------------------------------------------------
    void fill_bytes(std::byte value, std::size_t count) const {
        check_bounds(count);
        std::memset(data_ + position_, static_cast<int>(value), count);
        position_ += count;
    }

    void zero_fill(std::size_t count) const {
        fill_bytes(std::byte{0}, count);
    }

    // ---------------------------------------------------------------------
    // reader view
    // ---------------------------------------------------------------------
    [[nodiscard]] Reader as_reader() const noexcept {
        return Reader(data_, size_);
    }
};

} // namespace bytestream

#endif // BYTESTREAM_WRITER_HPP
