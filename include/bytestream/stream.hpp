#ifndef BYTESTREAM_STREAM_HPP
#define BYTESTREAM_STREAM_HPP

#include <bytestream/reader.hpp>
#include <bytestream/writer.hpp>

namespace bytestream {

class Stream {
private:
    std::byte*          data_;
    std::size_t         size_;
    mutable std::size_t pos_;

public:
    constexpr Stream(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), pos_(0) {}

    Reader reader() const noexcept {
        return Reader(data_, size_);
    }

    Writer writer() const noexcept {
        return Writer(data_, size_);
    }

    std::size_t position() const noexcept {
        return pos_;
    }

    void seek(std::size_t p) const {
        pos_ = p;
    }

    void rewind() const noexcept {
        pos_ = 0;
    }
};

} // namespace bytestream

#endif // BYTESTREAM_STREAM_HPP
