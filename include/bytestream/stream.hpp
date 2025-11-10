// include/bytestream/stream.hpp
#ifndef BYTESTREAM_STREAM_HPP
#define BYTESTREAM_STREAM_HPP

#include <bytestream/config.hpp>

namespace bytestream {
// Forward declarations
class Reader;
class Writer;

/**
 * @brief Bidirectional view combining Reader and Writer functionality
 *
 * Provides both read and write access to binary data with optional
 * shared position tracking.
 */
class Stream {
private:
    std::byte* data_;
    std::size_t size_;
    mutable std::size_t position_;

public:
    constexpr Stream(void* data, std::size_t size) noexcept
        : data_(static_cast<std::byte*>(data)), size_(size), position_(0)
    {
    }

    // Get separate Reader and Writer views
    [[nodiscard]] Reader reader() const noexcept;

    [[nodiscard]] Writer writer() const noexcept;

    // Shared position management
    [[nodiscard]] constexpr std::size_t position() const noexcept
    {
        return position_;
    }

    void seek(std::size_t pos) const
    {
        if(pos > size_)
        {
            throw std::out_of_range(
                      "Seek position " + std::to_string(pos) +
                      " exceeds size " + std::to_string(size_)
                      );
        }
        position_ = pos;
    }

    constexpr void rewind() const noexcept
    {
        position_ = 0;
    }
};
} // namespace bytestream

#endif // BYTESTREAM_STREAM_HPP
