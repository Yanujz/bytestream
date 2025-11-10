#ifndef BYTESTREAM_READER_HPP
#define BYTESTREAM_READER_HPP

#include <bytestream/config.hpp>
#include <cstring>

namespace bytestream {

class Reader {
private:
    const std::byte*    data_;
    std::size_t         size_;
    mutable std::size_t pos_;

    void check(std::size_t n) const {
        if (pos_ + n > size_) {
            throw UnderflowException("read past end");
        }
    }

public:
    constexpr Reader() noexcept
        : data_(nullptr), size_(0), pos_(0) {}

    constexpr Reader(const void* data, std::size_t size) noexcept
        : data_(static_cast<const std::byte*>(data)), size_(size), pos_(0) {}

    constexpr Reader(bytestream::span<const std::byte> s) noexcept
        : data_(s.data()), size_(s.size()), pos_(0) {}

    template<typename T>
    constexpr Reader(bytestream::span<const T> s) noexcept
        : data_(reinterpret_cast<const std::byte*>(s.data())),
          size_(s.size_bytes()),
          pos_(0) {}

    // also allow non-const span<T>
    template<typename T>
    constexpr Reader(bytestream::span<T> s) noexcept
        : data_(reinterpret_cast<const std::byte*>(s.data())),
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

    void align(std::size_t alignment) const {
        assert(alignment > 0 && detail::has_single_bit(alignment));
        std::size_t aligned = (pos_ + alignment - 1) & ~(alignment - 1);
        seek(aligned);
    }

    bool is_aligned(std::size_t alignment) const noexcept {
        return (pos_ % alignment) == 0;
    }

    Reader subview(std::size_t offset, std::size_t length = static_cast<std::size_t>(-1)) const {
        if (offset > size_) {
            throw std::out_of_range("subview offset");
        }
        std::size_t len = (length == static_cast<std::size_t>(-1)) ? (size_ - offset) : length;
        if (offset + len > size_) {
            throw std::out_of_range("subview length");
        }
        return Reader(data_ + offset, len);
    }

    template<typename T>
    T read() const {
        static_assert(detail::is_arithmetic<T>::value, "T must be arithmetic (not bool/char)");
        check(sizeof(T));
        T v;
        std::memcpy(&v, data_ + pos_, sizeof(T));
        pos_ += sizeof(T);
        return v;
    }

    template<typename T>
    T read_le() const {
        T v = read<T>();
        if constexpr (sizeof(T) > 1) {
            if (!is_little_endian()) {
                v = byteswap(v);
            }
        }
        return v;
    }

    template<typename T>
    T read_be() const {
        T v = read<T>();
        if constexpr (sizeof(T) > 1) {
            if (is_little_endian()) {
                v = byteswap(v);
            }
        }
        return v;
    }

    template<typename T>
    T peek() const {
        auto old = pos_;
        T v = read<T>();
        pos_ = old;
        return v;
    }

    template<typename T>
    T peek_le() const {
        auto old = pos_;
        T v = read_le<T>();
        pos_ = old;
        return v;
    }

    template<typename T>
    T peek_be() const {
        auto old = pos_;
        T v = read_be<T>();
        pos_ = old;
        return v;
    }

    void read_bytes(bytestream::span<std::byte> dst) const {
        check(dst.size());
        std::memcpy(dst.data(), data_ + pos_, dst.size());
        pos_ += dst.size();
    }

    void read_bytes(void* dst, std::size_t n) const {
        check(n);
        std::memcpy(dst, data_ + pos_, n);
        pos_ += n;
    }

    template<typename T>
    void read_array(bytestream::span<T> dst) const {
        static_assert(detail::is_arithmetic<T>::value, "T must be arithmetic");
        check(dst.size_bytes());
        std::memcpy(dst.data(), data_ + pos_, dst.size_bytes());
        pos_ += dst.size_bytes();
    }

    template<typename T>
    void read_array_le(bytestream::span<T> dst) const {
        if constexpr (is_little_endian() || sizeof(T) == 1) {
            read_array(dst);
        } else {
            for (auto &x : dst) {
                x = read_le<T>();
            }
        }
    }

    template<typename T>
    void read_array_be(bytestream::span<T> dst) const {
        if constexpr (is_big_endian() || sizeof(T) == 1) {
            read_array(dst);
        } else {
            for (auto &x : dst) {
                x = read_be<T>();
            }
        }
    }

    std::string read_string(std::size_t len) const {
        check(len);
        std::string s(len, '\0');
        std::memcpy(s.data(), data_ + pos_, len);
        pos_ += len;
        return s;
    }

    std::string read_sized_string_le() const {
        std::uint32_t len = read_le<std::uint32_t>();
        return read_string(len);
    }

    std::string read_sized_string_be() const {
        std::uint32_t len = read_be<std::uint32_t>();
        return read_string(len);
    }

    std::string read_cstring() const {
        const char* start = reinterpret_cast<const char*>(data_ + pos_);
        const char* end   = static_cast<const char*>(std::memchr(start, 0, remaining()));
        if (!end) {
            throw UnderflowException("no null terminator");
        }
        std::size_t len = static_cast<std::size_t>(end - start);
        std::string s(start, len);
        pos_ += len + 1;
        return s;
    }

    std::string_view view_string(std::size_t len) const {
        check(len);
        std::string_view v(reinterpret_cast<const char*>(data_ + pos_), len);
        pos_ += len;
        return v;
    }
};

} // namespace bytestream

#endif // BYTESTREAM_READER_HPP
