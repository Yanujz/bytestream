// include/bytestream/core.hpp
#ifndef BYTESTREAM_CORE_HPP
#define BYTESTREAM_CORE_HPP

#include <bytestream/config.hpp>
#include <bytestream/reader.hpp>
#include <bytestream/writer.hpp>
#include <bytestream/stream.hpp>

namespace bytestream {

inline Reader Stream::reader() const noexcept {
    return Reader(data_, size_);
}

inline Writer Stream::writer() const noexcept {
    return Writer(data_, size_);
}

} // namespace bytestream

#endif // BYTESTREAM_CORE_HPP