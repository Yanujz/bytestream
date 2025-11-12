#ifndef BYTESTREAM_CONFIG_HPP
#define BYTESTREAM_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <limits>
#include <cassert>

namespace bytestream {

// -------------------------------------------------------------
// Exceptions
// -------------------------------------------------------------
struct OverflowException  : std::runtime_error { using std::runtime_error::runtime_error; };
struct UnderflowException : std::runtime_error { using std::runtime_error::runtime_error; };

// -------------------------------------------------------------
// Debug-only assertion (no-ops in release unless you override)
// -------------------------------------------------------------
#ifndef BYTESTREAM_ASSERT
#  ifndef NDEBUG
#    define BYTESTREAM_ASSERT(expr) assert(expr)
#  else
#    define BYTESTREAM_ASSERT(expr) ((void)0)
#  endif
#endif

// -------------------------------------------------------------
// span (C++17-friendly)
// -------------------------------------------------------------
template <class T>
class span {
    T*          ptr_ = nullptr;
    std::size_t n_   = 0;
public:
    using element_type = T;
    span() = default;
    span(T* p, std::size_t n) : ptr_(p), n_(n) {}
    T* data() const noexcept { return ptr_; }
    std::size_t size() const noexcept { return n_; }
};

// -------------------------------------------------------------
// Endianness + byteswap
// -------------------------------------------------------------
enum class endian { little, big
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    , native = (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ? little : big
#elif defined(_WIN32)
    , native = little
#else
    , native = little
#endif
};

inline constexpr bool is_little_endian() noexcept { return endian::native == endian::little; }
inline constexpr bool is_big_endian()    noexcept { return endian::native == endian::big; }

inline std::uint16_t bswap16(std::uint16_t v) noexcept {
    return static_cast<std::uint16_t>((v >> 8) | (v << 8));
}
inline std::uint32_t bswap32(std::uint32_t v) noexcept {
    return  (v >> 24) |
           ((v & 0x00FF0000u) >> 8) |
           ((v & 0x0000FF00u) << 8) |
            (v << 24);
}
inline std::uint64_t bswap64(std::uint64_t v) noexcept {
    return  (v >> 56) |
           ((v & 0x00FF000000000000ull) >> 40) |
           ((v & 0x0000FF0000000000ull) >> 24) |
           ((v & 0x000000FF00000000ull) >> 8 ) |
           ((v & 0x00000000FF000000ull) << 8 ) |
           ((v & 0x0000000000FF0000ull) << 24) |
           ((v & 0x000000000000FF00ull) << 40) |
            (v << 56);
}

template <typename T>
inline T byteswap(T v) noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "byteswap requires trivially copyable type");
    if constexpr (std::is_same<T, std::uint16_t>::value || std::is_same<T, std::int16_t>::value) {
        auto u = static_cast<std::uint16_t>(v);
        u = bswap16(u);
        return static_cast<T>(u);
    } else if constexpr (std::is_same<T, std::uint32_t>::value || std::is_same<T, std::int32_t>::value) {
        auto u = static_cast<std::uint32_t>(v);
        u = bswap32(u);
        return static_cast<T>(u);
    } else if constexpr (std::is_same<T, std::uint64_t>::value || std::is_same<T, std::int64_t>::value) {
        auto u = static_cast<std::uint64_t>(v);
        u = bswap64(u);
        return static_cast<T>(u);
    } else if constexpr (std::is_floating_point<T>::value) {
        using U = typename std::conditional<sizeof(T)==4, std::uint32_t, std::uint64_t>::type;
        U u{};
        std::memcpy(&u, &v, sizeof(T));
        if constexpr (sizeof(T) == 4) u = bswap32(u); else u = bswap64(u);
        std::memcpy(&v, &u, sizeof(T));
        return v;
    } else {
        return v;
    }
}

// -------------------------------------------------------------
template <typename T>
struct is_arithmetic : std::integral_constant<bool,
    std::is_integral<T>::value || std::is_floating_point<T>::value> {};

inline std::size_t align_up(std::size_t p, std::size_t a) noexcept {
    if (a==0) return p;
    std::size_t r = p % a;
    return r ? (p + (a - r)) : p;
}

} // namespace bytestream

#endif // BYTESTREAM_CONFIG_HPP
