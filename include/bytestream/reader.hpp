#ifndef BYTESTREAM_READER_HPP
#define BYTESTREAM_READER_HPP

#include <bytestream/config.hpp>
#include <cstring>
#include <string>
#include <string_view>

namespace bytestream {

class Reader {
    const std::byte* data_;
    std::size_t      size_;
    std::size_t      pos_;
public:
    Reader(const void* data, std::size_t size) noexcept
        : data_(static_cast<const std::byte*>(data)), size_(size), pos_(0) {}

    template <class T>
    explicit Reader(span<T> s) noexcept
        : data_(reinterpret_cast<const std::byte*>(s.data())), size_(s.size()*sizeof(T)), pos_(0) {}

    std::size_t size() const noexcept { return size_; }
    std::size_t position() const noexcept { return pos_; }
    std::size_t remaining() const noexcept { return size_ - pos_; }
    bool empty() const noexcept { return size_ == 0; }
    bool exhausted() const noexcept { return pos_ >= size_; }
    const std::byte* data() const noexcept { return data_; }
    // Zero-copy peek at all bytes left to read
    span<const std::byte> remaining_bytes_view() const noexcept {
        return { data_ + pos_, size_ - pos_ };
    }

    void ensure(std::size_t n) const {
        if (n > (size_ - pos_)) throw UnderflowException("bytestream::Reader underflow");
    }

    void seek(std::size_t p) {
        if (p > size_) throw std::out_of_range("bytestream::Reader seek past end");
        pos_ = p;
    }
    void rewind() noexcept { pos_ = 0; }
    void skip(std::size_t n) { ensure(n); pos_ += n; }

    bool is_aligned(std::size_t a) const noexcept { return a == 0 || (pos_ % a) == 0; }
    void align(std::size_t a) {
        std::size_t next = align_up(pos_, a);
        if (next > size_) throw UnderflowException("bytestream::Reader align beyond end");
        pos_ = next;
    }

    // ---- raw bytes
    void read_bytes(void* dst, std::size_t n) {
        ensure(n);
        std::memcpy(dst, data_ + pos_, n);
        pos_ += n;
    }
    void read_bytes(span<std::byte> out) { read_bytes(out.data(), out.size()); }

    // ---- trivially-copyable read
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable<T>::value, T>
    read() {
        T v{};
        read_bytes(&v, sizeof(T));
        return v;
    }

    // ---- arithmetic endian-aware
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, T>
    read_le() {
        T v = read<T>();
        if constexpr (is_big_endian()) v = byteswap(v);
        return v;
    }
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, T>
    read_be() {
        T v = read<T>();
        if constexpr (is_little_endian()) v = byteswap(v);
        return v;
    }

    // ---- peek (no advance)
    template <typename T>
    T peek() {
        auto saved = pos_;
        T v = read<T>();
        pos_ = saved;
        return v;
    }
    template <typename T>
    T peek_le() {
        auto saved = pos_;
        T v = read_le<T>();
        pos_ = saved;
        return v;
    }

    // ---- arrays
    template <typename T>
    void read_array(span<T> out) {
        read_bytes(out.data(), out.size() * sizeof(T));
    }
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, void>
    read_array_be(span<T> out) {
        for (std::size_t i = 0; i < out.size(); ++i) out.data()[i] = read_be<T>();
    }
    template <typename T>
    std::enable_if_t<is_arithmetic<T>::value, void>
    read_array_le(span<T> out) {
        for (std::size_t i = 0; i < out.size(); ++i) out.data()[i] = read_le<T>();
    }

    // ---- strings
    std::string read_string(std::size_t n) {
        std::string s;
        s.resize(n);
        if (n) read_bytes(&s[0], n);
        return s;
    }
    std::string read_sized_string_le() {
        auto len = read_le<std::uint32_t>();
        return read_string(len);
    }
    std::string read_sized_string_be() {
        auto len = read_be<std::uint32_t>();
        return read_string(len);
    }
    std::string read_cstring() {
        // scan for 0 byte
        std::size_t i = pos_;
        for (; i < size_; ++i) {
            if (data_[i] == std::byte{0}) break;
        }
        if (i == size_) throw UnderflowException("bytestream::Reader cstring unterminated");
        std::size_t n = i - pos_;
        std::string s = read_string(n);
        // consume terminator
        skip(1);
        return s;
    }
    std::string_view view_string(std::size_t n) {
        ensure(n);
        auto sv = std::string_view(reinterpret_cast<const char*>(data_ + pos_), n);
        pos_ += n;
        return sv;
    }

    // ---- subviews
    Reader subview(std::size_t offset, std::size_t length) const {
        if (offset > size_ || length > (size_ - offset)) throw std::out_of_range("bytestream::Reader subview OOB");
        return Reader{data_ + offset, length};
    }
    Reader subview(std::size_t offset) const {
        if (offset > size_) throw std::out_of_range("bytestream::Reader subview OOB");
        return Reader{data_ + offset, size_ - offset};
    }

    // ---- native helper used by serialization.hpp
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable<T>::value, T>
    read_native() { return read<T>(); }
};

} // namespace bytestream

#endif // BYTESTREAM_READER_HPP
