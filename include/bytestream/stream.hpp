#ifndef BYTESTREAM_STREAM_HPP
#define BYTESTREAM_STREAM_HPP

#include <bytestream/writer.hpp>
#include <bytestream/reader.hpp>

namespace bytestream {
class Stream {
    std::byte*  data_;
    std::size_t size_;
    mutable std::size_t pos_;
public:
    Stream(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), pos_(0) {}

    Writer writer() const noexcept { Writer w{data_, size_}; const_cast<Writer&>(w).seek(pos_); return w; }
    Reader reader() const noexcept { Reader r{data_, size_}; const_cast<Reader&>(r).seek(pos_); return r; }

    std::size_t position() const noexcept { return pos_; }
    void seek(std::size_t p) const { pos_ = p; }
    void rewind() const noexcept { pos_ = 0; }
};
} // namespace bytestream
#endif // BYTESTREAM_STREAM_HPP
