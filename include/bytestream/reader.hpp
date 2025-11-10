#ifndef BYTESTREAM_READER_HPP
#define BYTESTREAM_READER_HPP

#include <bytestream/config.hpp>
#include <limits>
#include <cstring>

namespace bytestream {

class Reader {
private:
    const std::byte*    data_;
    std::size_t         size_;
    mutable std::size_t position_;

    void check_bounds(std::size_t bytes) const {
        if (position_ + bytes > size_) {
            throw UnderflowException(
                "Attempted to read " + std::to_string(bytes) +
                " bytes at position " + std::to_string(position_) +
                " (size: " + std::to_string(size_) + ")"
            );
        }
    }

public:
    // ---------------------------------------------------------------------
    // ctors
    // ---------------------------------------------------------------------
    constexpr Reader() noexcept
        : data_(nullptr), size_(0), position_(0) {}

    constexpr Reader(const void* data, std::size_t size) noexcept
        : data_(static_cast<const std::byte*>(data)), size_(size), position_(0) {}

    // from span<const byte>
    constexpr Reader(bytestream::span<const std::byte> s) noexcept
        : data_(s.data()), size_(s.size()), position_(0) {}

    // from span<const T>
    template<typename T>
    constexpr Reader(bytestream::span<const T> s) noexcept
        : data_(reinterpret_cast<const std::byte*>(s.data())),
          size_(s.size_bytes()), position_(0) {}

    // from span<T>  (this fixes Reader reader(span<uint8_t>) in tests)
    template<typename T>
    constexpr Reader(bytestream::span<T> s) noexcept
        : data_(reinterpret_cast<const std::byte*>(s.data())),
          size_(s.size_bytes()), position_(0) {}

    // ---------------------------------------------------------------------
    // properties
    // ---------------------------------------------------------------------
    [[nodiscard]] constexpr const std::byte* data() const noexcept { return data_; }
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

    void align(std::size_t alignment) const {
        assert(alignment > 0 && detail::has_single_bit(alignment));
        std::size_t aligned = (position_ + alignment - 1) & ~(alignment - 1);
        seek(aligned);
    }

    [[nodiscard]] bool is_aligned(std::size_t alignment) const noexcept {
        return (position_ % alignment) == 0;
    }

    // ---------------------------------------------------------------------
    // subview
    // ---------------------------------------------------------------------
    [[nodiscard]] Reader subview(std::size_t offset,
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
        return Reader(data_ + offset, actual);
    }

    // ---------------------------------------------------------------------
    // arithmetic reads
    // ---------------------------------------------------------------------
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T read() const {
        check_bounds(sizeof(T));
        T value;
        std::memcpy(&value, data_ + position_, sizeof(T));
        position_ += sizeof(T);
        return value;
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T read_le() const {
        T v = read<T>();
        if constexpr (sizeof(T) > 1) {
            if constexpr (!is_little_endian()) {
                return byteswap(v);
            }
        }
        return v;
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T read_be() const {
        T v = read<T>();
        if constexpr (sizeof(T) > 1) {
            if constexpr (is_little_endian()) {
                return byteswap(v);
            }
        }
        return v;
    }

    // ---------------------------------------------------------------------
    // peek
    // ---------------------------------------------------------------------
    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T peek() const {
        auto saved = position_;
        T v = read<T>();
        position_ = saved;
        return v;
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T peek_le() const {
        auto saved = position_;
        T v = read_le<T>();
        position_ = saved;
        return v;
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    [[nodiscard]] T peek_be() const {
        auto saved = position_;
        T v = read_be<T>();
        position_ = saved;
        return v;
    }

    // ---------------------------------------------------------------------
    // bulk
    // ---------------------------------------------------------------------
    void read_bytes(bytestream::span<std::byte> dest) const {
        check_bounds(dest.size());
        std::memcpy(dest.data(), data_ + position_, dest.size());
        position_ += dest.size();
    }

    void read_bytes(void* dest, std::size_t count) const {
        check_bounds(count);
        std::memcpy(dest, data_ + position_, count);
        position_ += count;
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void read_array(bytestream::span<T> dest) const {
        check_bounds(dest.size_bytes());
        std::memcpy(dest.data(), data_ + position_, dest.size_bytes());
        position_ += dest.size_bytes();
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void read_array_le(bytestream::span<T> dest) const {
        if constexpr (is_little_endian() || sizeof(T) == 1) {
            read_array(dest);
        } else {
            for (auto &e : dest) {
                e = read_le<T>();
            }
        }
    }

    template<typename T, detail::enable_if_arithmetic_t<T> = 0>
    void read_array_be(bytestream::span<T> dest) const {
        if constexpr (is_big_endian() || sizeof(T) == 1) {
            read_array(dest);
        } else {
            for (auto &e : dest) {
                e = read_be<T>();
            }
        }
    }

    // ---------------------------------------------------------------------
    // strings
    // ---------------------------------------------------------------------
    [[nodiscard]] std::string read_string(std::size_t length) const {
        check_bounds(length);
        std::string res(length, '\0');
        std::memcpy(res.data(), data_ + position_, length);
        position_ += length;
        return res;
    }

    [[nodiscard]] std::string read_sized_string_le() const {
        uint32_t len = read_le<uint32_t>();
        return read_string(len);
    }

    [[nodiscard]] std::string read_sized_string_be() const {
        uint32_t len = read_be<uint32_t>();
        return read_string(len);
    }

    [[nodiscard]] std::string read_cstring() const {
        const char* start = reinterpret_cast<const char*>(data_ + position_);
        const char* end   = static_cast<const char*>(std::memchr(start, 0, remaining()));
        if (!end) {
            throw UnderflowException("No null terminator found");
        }
        std::size_t len = static_cast<std::size_t>(end - start);
        std::string res(start, len);
        position_ += len + 1;
        return res;
    }

    [[nodiscard]] std::string_view view_string(std::size_t length) const {
        check_bounds(length);
        std::string_view v(reinterpret_cast<const char*>(data_ + position_), length);
        position_ += length;
        return v;
    }
};

} // namespace bytestream

#endif // BYTESTREAM_READER_HPP
