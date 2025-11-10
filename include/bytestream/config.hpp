// include/bytestream/config.hpp
#ifndef BYTESTREAM_CONFIG_HPP
#define BYTESTREAM_CONFIG_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <string>
#include <string_view>
#include <cassert>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

// Detect language level
#if defined(_MSVC_LANG)
    #define BYTESTREAM_CPLUSPLUS _MSVC_LANG
#else
    #define BYTESTREAM_CPLUSPLUS __cplusplus
#endif

// C++20 or later?
#if BYTESTREAM_CPLUSPLUS >= 202002L
    #define BYTESTREAM_HAS_CPP20 1
#else
    #define BYTESTREAM_HAS_CPP20 0
#endif

// std::span?
#if BYTESTREAM_HAS_CPP20 && __has_include(<span>)
    #include <span>
    #define BYTESTREAM_HAS_STD_SPAN 1
#else
    #define BYTESTREAM_HAS_STD_SPAN 0
#endif

// std::endian?
#if BYTESTREAM_HAS_CPP20 && __has_include(<bit>)
    #include <bit>
    #define BYTESTREAM_HAS_STD_ENDIAN 1
#else
    #define BYTESTREAM_HAS_STD_ENDIAN 0
#endif

namespace bytestream {

// ============================================================================
// Version
// ============================================================================
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

// ============================================================================
// Exceptions
// ============================================================================
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message)
        : std::runtime_error(message) {}
};

class OverflowException : public Exception {
public:
    explicit OverflowException(const std::string& message)
        : Exception("Buffer overflow: " + message) {}
};

class UnderflowException : public Exception {
public:
    explicit UnderflowException(const std::string& message)
        : Exception("Buffer underflow: " + message) {}
};

class AlignmentException : public Exception {
public:
    explicit AlignmentException(const std::string& message)
        : Exception("Alignment error: " + message) {}
};

class AccessException : public Exception {
public:
    explicit AccessException(const std::string& message)
        : Exception("Access violation: " + message) {}
};

// ============================================================================
// span alias (C++17 fallback)
// ============================================================================
#if !BYTESTREAM_HAS_STD_SPAN
namespace detail {
    template<typename T>
    class span {
    public:
        using element_type = T;
        using value_type   = typename std::remove_cv<T>::type;
        using size_type    = std::size_t;
        using pointer      = T*;
        using iterator     = T*;

        constexpr span() noexcept : data_(nullptr), size_(0) {}
        constexpr span(T* ptr, size_type count) noexcept : data_(ptr), size_(count) {}
        
        template<std::size_t N>
        constexpr span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}
        
        template<typename Container,
                 typename = typename std::enable_if<
                     std::is_convertible<decltype(std::declval<Container>().data()), T*>::value &&
                     std::is_convertible<decltype(std::declval<Container>().size()), size_type>::value
                 >::type>
        constexpr span(Container& c) noexcept : data_(c.data()), size_(c.size()) {}

        constexpr pointer data() const noexcept { return data_; }
        constexpr size_type size() const noexcept { return size_; }
        constexpr size_type size_bytes() const noexcept { return size_ * sizeof(T); }
        constexpr bool empty() const noexcept { return size_ == 0; }

        constexpr iterator begin() const noexcept { return data_; }
        constexpr iterator end() const noexcept { return data_ + size_; }
        
        constexpr T& operator[](size_type idx) const noexcept { return data_[idx]; }

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

// ============================================================================
// Traits / helpers
// ============================================================================
namespace detail {

template<typename T>
struct is_arithmetic
    : std::integral_constant<bool,
                             std::is_arithmetic<T>::value &&
                                 !std::is_same<T, bool>::value &&
                                 !std::is_same<T, char>::value> {};
template<typename T>
constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

template<typename T>
struct is_integral
    : std::integral_constant<bool,
                             std::is_integral<T>::value &&
                                 !std::is_same<T, bool>::value> {};
template<typename T>
constexpr bool is_integral_v = is_integral<T>::value;

template<typename T>
using is_floating = std::is_floating_point<T>;
template<typename T>
constexpr bool is_floating_v = is_floating<T>::value;

template<typename T>
using enable_if_arithmetic_t = typename std::enable_if<is_arithmetic_v<T>, int>::type;
template<typename T>
using enable_if_integral_t   = typename std::enable_if<is_integral_v<T>, int>::type;
template<typename T>
using enable_if_floating_t   = typename std::enable_if<is_floating_v<T>, int>::type;

constexpr inline bool has_single_bit(std::size_t v) noexcept {
    return v && ((v & (v - 1)) == 0);
}

} // namespace detail

// ============================================================================
// Endianness
// ============================================================================
enum class Endian {
    Little = 0,
    Big    = 1,
#if BYTESTREAM_HAS_STD_ENDIAN
    Native = static_cast<int>(std::endian::native)
#else
    #if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
        Native = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ? Little : Big
    #elif defined(_WIN32)
        Native = Little
    #else
        Native = Little
    #endif
#endif
};

constexpr bool is_little_endian() noexcept {
#if BYTESTREAM_HAS_STD_ENDIAN
    return std::endian::native == std::endian::little;
#else
    #if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
        return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
    #elif defined(_WIN32)
        return true;
    #else
        return true;
    #endif
#endif
}

constexpr bool is_big_endian() noexcept {
    return !is_little_endian();
}

// ============================================================================
// byteswap
// ============================================================================
template<typename T, detail::enable_if_integral_t<T> = 0>
[[nodiscard]] constexpr T byteswap(T value) noexcept
{
    if (sizeof(T) == 1) {
        return value;
    }
    // std::byteswap in C++23:
#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
    return std::byteswap(value);
#else
    if (sizeof(T) == 2) {
        auto v = static_cast<uint16_t>(value);
        #if defined(_MSC_VER)
            return static_cast<T>(_byteswap_ushort(v));
        #elif defined(__GNUC__) || defined(__clang__)
            return static_cast<T>(__builtin_bswap16(v));
        #else
            return static_cast<T>((v >> 8) | (v << 8));
        #endif
    } else if (sizeof(T) == 4) {
        auto v = static_cast<uint32_t>(value);
        #if defined(_MSC_VER)
            return static_cast<T>(_byteswap_ulong(v));
        #elif defined(__GNUC__) || defined(__clang__)
            return static_cast<T>(__builtin_bswap32(v));
        #else
            return static_cast<T>(
                ((v & 0xFF000000) >> 24) |
                ((v & 0x00FF0000) >> 8)  |
                ((v & 0x0000FF00) << 8)  |
                ((v & 0x000000FF) << 24));
        #endif
    } else if (sizeof(T) == 8) {
        auto v = static_cast<uint64_t>(value);
        #if defined(_MSC_VER)
            return static_cast<T>(_byteswap_uint64(v));
        #elif defined(__GNUC__) || defined(__clang__)
            return static_cast<T>(__builtin_bswap64(v));
        #else
            return static_cast<T>(
                ((v & 0xFF00000000000000ULL) >> 56) |
                ((v & 0x00FF000000000000ULL) >> 40) |
                ((v & 0x0000FF0000000000ULL) >> 24) |
                ((v & 0x000000FF00000000ULL) >> 8)  |
                ((v & 0x00000000FF000000ULL) << 8)  |
                ((v & 0x0000000000FF0000ULL) << 24) |
                ((v & 0x000000000000FF00ULL) << 40) |
                ((v & 0x00000000000000FFULL) << 56));
        #endif
    }
    return value;
#endif
}

template<typename T, detail::enable_if_floating_t<T> = 0>
[[nodiscard]] inline T byteswap(T value) noexcept
{
    if (sizeof(T) == 4) {
        uint32_t temp;
        std::memcpy(&temp, &value, sizeof(T));
        temp = byteswap(temp);
        std::memcpy(&value, &temp, sizeof(T));
    } else if (sizeof(T) == 8) {
        uint64_t temp;
        std::memcpy(&temp, &value, sizeof(T));
        temp = byteswap(temp);
        std::memcpy(&value, &temp, sizeof(T));
    }
    return value;
}

} // namespace bytestream

#endif // BYTESTREAM_CONFIG_HPP