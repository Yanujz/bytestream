#ifndef BYTESTREAM_CONFIG_HPP
#define BYTESTREAM_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <string>
#include <string_view>
#include <cassert>

#if defined(_MSC_VER)
#  include <intrin.h>
#endif

// detect language level
#if defined(_MSVC_LANG)
#  define BYTESTREAM_CXX _MSVC_LANG
#else
#  define BYTESTREAM_CXX __cplusplus
#endif

// std::span available?
#if BYTESTREAM_CXX >= 202002L && __has_include(<span>)
#  include <span>
#  define BYTESTREAM_HAS_STD_SPAN 1
#else
#  define BYTESTREAM_HAS_STD_SPAN 0
#endif

namespace bytestream {

// version (kept small)
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

// exceptions
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string &msg) : std::runtime_error(msg) {}
};

class OverflowException : public Exception {
public:
    explicit OverflowException(const std::string &msg)
        : Exception("Buffer overflow: " + msg) {}
};

class UnderflowException : public Exception {
public:
    explicit UnderflowException(const std::string &msg)
        : Exception("Buffer underflow: " + msg) {}
};

class AlignmentException : public Exception {
public:
    explicit AlignmentException(const std::string &msg)
        : Exception("Alignment error: " + msg) {}
};

class AccessException : public Exception {
public:
    explicit AccessException(const std::string &msg)
        : Exception("Access violation: " + msg) {}
};

// span fallback
#if !BYTESTREAM_HAS_STD_SPAN
namespace detail {
    template<typename T>
    class span {
    public:
        using element_type = T;
        using size_type    = std::size_t;

        constexpr span() noexcept : data_(nullptr), size_(0) {}
        constexpr span(T* ptr, size_type n) noexcept : data_(ptr), size_(n) {}
        template<std::size_t N>
        constexpr span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}

        constexpr T*          data() const noexcept { return data_; }
        constexpr size_type   size() const noexcept { return size_; }
        constexpr size_type   size_bytes() const noexcept { return size_ * sizeof(T); }
        constexpr bool        empty() const noexcept { return size_ == 0; }

        constexpr T*          begin() const noexcept { return data_; }
        constexpr T*          end() const noexcept { return data_ + size_; }

    private:
        T*        data_;
        size_type size_;
    };
} // namespace detail

template<typename T>
using span = detail::span<T>;
#else
template<typename T>
using span = std::span<T>;
#endif

// small trait helpers
namespace detail {
    template<typename T>
    using is_arithmetic =
        std::bool_constant<std::is_arithmetic<T>::value &&
                           !std::is_same<T, bool>::value &&
                           !std::is_same<T, char>::value>;

    template<typename T>
    using is_integral =
        std::bool_constant<std::is_integral<T>::value &&
                           !std::is_same<T, bool>::value>;

    template<typename T>
    using is_floating = std::is_floating_point<T>;

    constexpr inline bool has_single_bit(std::size_t v) noexcept {
        return v && ((v & (v - 1)) == 0);
    }
}

// endianness
enum class Endian {
    Little = 0,
    Big    = 1,
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    Native = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ? Little : Big
#elif defined(_WIN32)
    Native = Little
#else
    Native = Little
#endif
};

constexpr bool is_little_endian() noexcept {
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#elif defined(_WIN32)
    return true;
#else
    return true;
#endif
}

constexpr bool is_big_endian() noexcept {
    return !is_little_endian();
}

// byteswap
template<typename T>
inline T byteswap_int(T value) noexcept {
    if constexpr (sizeof(T) == 2) {
        auto v = static_cast<std::uint16_t>(value);
    #if defined(_MSC_VER)
        v = _byteswap_ushort(v);
    #elif defined(__GNUC__) || defined(__clang__)
        v = __builtin_bswap16(v);
    #else
        v = static_cast<std::uint16_t>((v >> 8) | (v << 8));
    #endif
        return static_cast<T>(v);
    } else if constexpr (sizeof(T) == 4) {
        auto v = static_cast<std::uint32_t>(value);
    #if defined(_MSC_VER)
        v = _byteswap_ulong(v);
    #elif defined(__GNUC__) || defined(__clang__)
        v = __builtin_bswap32(v);
    #else
        v = ((v & 0xFF000000u) >> 24) |
            ((v & 0x00FF0000u) >> 8 ) |
            ((v & 0x0000FF00u) << 8 ) |
            ((v & 0x000000FFu) << 24);
    #endif
        return static_cast<T>(v);
    } else if constexpr (sizeof(T) == 8) {
        auto v = static_cast<std::uint64_t>(value);
    #if defined(_MSC_VER)
        v = _byteswap_uint64(v);
    #elif defined(__GNUC__) || defined(__clang__)
        v = __builtin_bswap64(v);
    #else
        v = ((v & 0xFF00000000000000ULL) >> 56) |
            ((v & 0x00FF000000000000ULL) >> 40) |
            ((v & 0x0000FF0000000000ULL) >> 24) |
            ((v & 0x000000FF00000000ULL) >> 8 ) |
            ((v & 0x00000000FF000000ULL) << 8 ) |
            ((v & 0x0000000000FF0000ULL) << 24) |
            ((v & 0x000000000000FF00ULL) << 40) |
            ((v & 0x00000000000000FFULL) << 56);
    #endif
        return static_cast<T>(v);
    } else {
        return value;
    }
}

template<typename T>
inline T byteswap(T value) noexcept {
    if constexpr (detail::is_integral<T>::value) {
        return byteswap_int(value);
    } else if constexpr (detail::is_floating<T>::value) {
        if constexpr (sizeof(T) == 4) {
            std::uint32_t tmp;
            std::memcpy(&tmp, &value, 4);
            tmp = byteswap_int(tmp);
            std::memcpy(&value, &tmp, 4);
        } else if constexpr (sizeof(T) == 8) {
            std::uint64_t tmp;
            std::memcpy(&tmp, &value, 8);
            tmp = byteswap_int(tmp);
            std::memcpy(&value, &tmp, 8);
        }
        return value;
    } else {
        return value;
    }
}

} // namespace bytestream

#endif // BYTESTREAM_CONFIG_HPP
