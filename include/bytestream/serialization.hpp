#ifndef BYTESTREAM_SERIALIZATION_HPP
#define BYTESTREAM_SERIALIZATION_HPP

#include <bytestream/config.hpp>
#include <bytestream/reader.hpp>
#include <bytestream/writer.hpp>
#include <type_traits>
#include <vector>
#include <array>
#include <string>

namespace bytestream {

// ------------------------------------------------------------------
// CRTP base (first so traits can see it)
// ------------------------------------------------------------------
template <typename Derived>
struct Serializable {
    void serialize(Writer& w) const {
        static_cast<const Derived*>(this)->serialize_impl(w);
    }
    void deserialize_impl(Reader&); // defined by Derived
    static Derived deserialize(Reader& r) {
        Derived d{};
        d.deserialize_impl(r);
        return d;
    }
};

// ------------------------------------------------------------------
// Traits (detail) — CRTP-aware
// ------------------------------------------------------------------
namespace detail {

template <typename T, typename = void>
struct has_serialize_method : std::false_type {};
template <typename T>
struct has_serialize_method<T, std::void_t<decltype(std::declval<const T&>().serialize(std::declval<Writer&>()))>> : std::true_type {};

template <typename T, typename = void>
struct has_deserialize_static : std::false_type {};
template <typename T>
struct has_deserialize_static<T, std::void_t<decltype(T::deserialize(std::declval<Reader&>()))>> : std::true_type {};

template <typename T>
struct is_trivially_serializable : std::integral_constant<bool, std::is_trivially_copyable<T>::value> {};
template <typename T>
inline constexpr bool is_trivially_serializable_v = is_trivially_serializable<T>::value;

template <typename T>
struct is_crtp_serializable : std::is_base_of<Serializable<T>, T> {};

template <typename T>
struct is_serializable : std::integral_constant<bool,
    has_serialize_method<T>::value || has_deserialize_static<T>::value || is_crtp_serializable<T>::value> {};
template <typename T>
inline constexpr bool is_serializable_v = is_serializable<T>::value;

// helper for static_assert fallthrough
template <class> struct dependent_false : std::false_type {};

} // namespace detail

// ------------------------------------------------------------------
// Single-dispatch write_field (no overload ambiguity)
// ------------------------------------------------------------------
template <typename T>
void write_field(Writer& w, const T& v) {
    if constexpr (std::is_same_v<T, std::string>) {
        w.write_sized_string_le(v);
    } else if constexpr (detail::has_serialize_method<T>::value) {
        // custom serializable (incl. CRTP types via serialize_impl)
        v.serialize(w);
    } else if constexpr (detail::is_trivially_serializable_v<T> && !detail::is_serializable_v<T>) {
        // plain POD/trivial types with no custom serialize/deserialize
        w.write_native(v);
    } else {
        static_assert(detail::dependent_false<T>::value, "write_field: unsupported type T");
    }
}

// convenience variadic
template <typename... Ts>
void write_fields(Writer& w, const Ts&... ts) { (write_field(w, ts), ...); }

// endian-explicit for arithmetic
template <typename T>
std::enable_if_t<is_arithmetic<T>::value, void>
write_field_le(Writer& w, T v) { w.write_le<T>(v); }

template <typename T>
std::enable_if_t<is_arithmetic<T>::value, void>
write_field_be(Writer& w, T v) { w.write_be<T>(v); }

// vectors/arrays
template <typename T>
void write_vector(Writer& w, const std::vector<T>& v) {
    w.write_le<std::uint32_t>(static_cast<std::uint32_t>(v.size()));
    for (const auto& x : v) write_field(w, x);
}
template <typename T, std::size_t N>
void write_array(Writer& w, const std::array<T, N>& a) {
    for (const auto& x : a) write_field(w, x);
}

// ------------------------------------------------------------------
// Single-dispatch read_field (no overload ambiguity)
// ------------------------------------------------------------------
template <typename T>
T read_field(Reader& r) {
    if constexpr (std::is_same_v<T, std::string>) {
        return r.read_sized_string_le();
    } else if constexpr (detail::has_deserialize_static<T>::value || detail::is_crtp_serializable<T>::value) {
        // custom serializable (incl. CRTP types via T::deserialize)
        return T::deserialize(r);
    } else if constexpr (detail::is_trivially_serializable_v<T> && !detail::is_serializable_v<T>) {
        // plain POD/trivial types with no custom serialize/deserialize
        return r.read_native<T>();
    } else {
        static_assert(detail::dependent_false<T>::value, "read_field: unsupported type T");
    }
}

// endian-explicit for arithmetic
template <typename T>
std::enable_if_t<is_arithmetic<T>::value, T>
read_field_le(Reader& r) { return r.read_le<T>(); }

template <typename T>
std::enable_if_t<is_arithmetic<T>::value, T>
read_field_be(Reader& r) { return r.read_be<T>(); }

// vectors/arrays
template <typename T>
std::vector<T> read_vector(Reader& r) {
    std::uint32_t n = r.read_le<std::uint32_t>();
    std::vector<T> out;
    out.reserve(n);
    for (std::uint32_t i = 0; i < n; ++i) out.push_back(read_field<T>(r));
    return out;
}
template <typename T, std::size_t N>
std::array<T, N> read_array(Reader& r) {
    std::array<T, N> a{};
    for (auto& x : a) x = read_field<T>(r);
    return a;
}

} // namespace bytestream

#endif // BYTESTREAM_SERIALIZATION_HPP
