// tests/test_reader.cpp
#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <vector>
#include <array>
#include <cstring>

using namespace bytestream;

class ReaderTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buffer;
    
    void SetUp() override {
        buffer.resize(1024, 0);
    }
};

// ============================================================================
// Basic Operations
// ============================================================================

TEST_F(ReaderTest, Construction) {
    Reader reader(buffer.data(), buffer.size());
    
    EXPECT_EQ(reader.size(), 1024);
    EXPECT_EQ(reader.position(), 0);
    EXPECT_EQ(reader.remaining(), 1024);
    EXPECT_FALSE(reader.empty());
    EXPECT_FALSE(reader.exhausted());
}

TEST_F(ReaderTest, EmptyBuffer) {
    Reader reader(nullptr, 0);
    
    EXPECT_EQ(reader.size(), 0);
    EXPECT_TRUE(reader.empty());
    EXPECT_TRUE(reader.exhausted());
}

TEST_F(ReaderTest, SpanConstruction) {
    bytestream::span<uint8_t> span(buffer.data(), buffer.size());
    Reader reader(span);
    
    EXPECT_EQ(reader.size(), buffer.size());
}

// ============================================================================
// Position Management
// ============================================================================

TEST_F(ReaderTest, Seek) {
    Reader reader(buffer.data(), buffer.size());
    
    reader.seek(100);
    EXPECT_EQ(reader.position(), 100);
    EXPECT_EQ(reader.remaining(), 924);
    
    reader.seek(0);
    EXPECT_EQ(reader.position(), 0);
    
    reader.seek(1024);
    EXPECT_EQ(reader.position(), 1024);
    EXPECT_TRUE(reader.exhausted());
}

TEST_F(ReaderTest, SeekOutOfBounds) {
    Reader reader(buffer.data(), buffer.size());
    
    EXPECT_THROW(reader.seek(1025), std::out_of_range);
}

TEST_F(ReaderTest, Rewind) {
    Reader reader(buffer.data(), buffer.size());
    
    reader.seek(500);
    reader.rewind();
    
    EXPECT_EQ(reader.position(), 0);
}

TEST_F(ReaderTest, Skip) {
    Reader reader(buffer.data(), buffer.size());
    
    reader.skip(10);
    EXPECT_EQ(reader.position(), 10);
    
    reader.skip(100);
    EXPECT_EQ(reader.position(), 110);
}

TEST_F(ReaderTest, SkipBeyondEnd) {
    Reader reader(buffer.data(), 10);
    
    EXPECT_THROW(reader.skip(11), UnderflowException);
}

// ============================================================================
// Alignment
// ============================================================================

TEST_F(ReaderTest, Alignment) {
    Reader reader(buffer.data(), buffer.size());
    
    reader.seek(5);
    EXPECT_FALSE(reader.is_aligned(4));
    
    reader.align(4);
    EXPECT_EQ(reader.position(), 8);
    EXPECT_TRUE(reader.is_aligned(4));
    
    reader.align(16);
    EXPECT_EQ(reader.position(), 16);
    EXPECT_TRUE(reader.is_aligned(16));
}

// ============================================================================
// Reading Primitive Types
// ============================================================================

TEST_F(ReaderTest, ReadUint8) {
    buffer[0] = 0x42;
    Reader reader(buffer.data(), buffer.size());
    
    uint8_t value = reader.read<uint8_t>();
    EXPECT_EQ(value, 0x42);
    EXPECT_EQ(reader.position(), 1);
}

TEST_F(ReaderTest, ReadUint16LE) {
    buffer[0] = 0x34;
    buffer[1] = 0x12;
    Reader reader(buffer.data(), buffer.size());
    
    uint16_t value = reader.read_le<uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 2);
}

TEST_F(ReaderTest, ReadUint16BE) {
    buffer[0] = 0x12;
    buffer[1] = 0x34;
    Reader reader(buffer.data(), buffer.size());
    
    uint16_t value = reader.read_be<uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 2);
}

TEST_F(ReaderTest, ReadUint32LE) {
    buffer[0] = 0x78;
    buffer[1] = 0x56;
    buffer[2] = 0x34;
    buffer[3] = 0x12;
    Reader reader(buffer.data(), buffer.size());
    
    uint32_t value = reader.read_le<uint32_t>();
    EXPECT_EQ(value, 0x12345678);
    EXPECT_EQ(reader.position(), 4);
}

TEST_F(ReaderTest, ReadUint32BE) {
    buffer[0] = 0x12;
    buffer[1] = 0x34;
    buffer[2] = 0x56;
    buffer[3] = 0x78;
    Reader reader(buffer.data(), buffer.size());
    
    uint32_t value = reader.read_be<uint32_t>();
    EXPECT_EQ(value, 0x12345678);
    EXPECT_EQ(reader.position(), 4);
}

TEST_F(ReaderTest, ReadUint64LE) {
    for (size_t i = 0; i < 8; ++i) {
        buffer[i] = static_cast<uint8_t>(0x88 - i * 0x11);
    }
    Reader reader(buffer.data(), buffer.size());
    
    uint64_t value = reader.read_le<uint64_t>();
    EXPECT_EQ(value, 0x1122334455667788ULL);
    EXPECT_EQ(reader.position(), 8);
}

TEST_F(ReaderTest, ReadInt32) {
    int32_t test_val = -12345;
    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    
    Reader reader(buffer.data(), buffer.size());
    int32_t value = reader.read<int32_t>();
    
    EXPECT_EQ(value, test_val);
}

TEST_F(ReaderTest, ReadFloat) {
    float test_val = 3.14159f;
    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    
    Reader reader(buffer.data(), buffer.size());
    float value = reader.read<float>();
    
    EXPECT_FLOAT_EQ(value, test_val);
}

TEST_F(ReaderTest, ReadDouble) {
    double test_val = 2.718281828459045;
    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    
    Reader reader(buffer.data(), buffer.size());
    double value = reader.read<double>();
    
    EXPECT_DOUBLE_EQ(value, test_val);
}

// ============================================================================
// Peek Operations
// ============================================================================

TEST_F(ReaderTest, Peek) {
    buffer[0] = 0x42;
    buffer[1] = 0x43;
    Reader reader(buffer.data(), buffer.size());
    
    uint8_t value1 = reader.peek<uint8_t>();
    EXPECT_EQ(value1, 0x42);
    EXPECT_EQ(reader.position(), 0);
    
    uint8_t value2 = reader.peek<uint8_t>();
    EXPECT_EQ(value2, 0x42);
    EXPECT_EQ(reader.position(), 0);
    
    uint8_t value3 = reader.read<uint8_t>();
    EXPECT_EQ(value3, 0x42);
    EXPECT_EQ(reader.position(), 1);
}

TEST_F(ReaderTest, PeekLE) {
    buffer[0] = 0x34;
    buffer[1] = 0x12;
    Reader reader(buffer.data(), buffer.size());
    
    uint16_t value = reader.peek_le<uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 0);
}

// ============================================================================
// Bulk Operations
// ============================================================================

TEST_F(ReaderTest, ReadBytes) {
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<uint8_t>(i);
    }
    
    Reader reader(buffer.data(), buffer.size());
    std::array<uint8_t, 10> dest{};
    
    reader.read_bytes(dest.data(), 10);
    
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(dest[i], i);
    }
    EXPECT_EQ(reader.position(), 10);
}

TEST_F(ReaderTest, ReadBytesSpan) {
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<uint8_t>(i);
    }
    
    Reader reader(buffer.data(), buffer.size());
    std::array<std::byte, 10> dest{};
    
    reader.read_bytes(bytestream::span<std::byte>(dest.data(), dest.size()));
    
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(static_cast<uint8_t>(dest[i]), i);
    }
}

TEST_F(ReaderTest, ReadArray) {
    std::array<uint16_t, 5> test_data = {{1, 2, 3, 4, 5}};
    std::memcpy(buffer.data(), test_data.data(), sizeof(test_data));
    
    Reader reader(buffer.data(), buffer.size());
    std::array<uint16_t, 5> dest{};
    
    reader.read_array(bytestream::span<uint16_t>(dest.data(), dest.size()));
    
    EXPECT_EQ(dest, test_data);
    EXPECT_EQ(reader.position(), 10);
}

TEST_F(ReaderTest, ReadArrayLE) {
    // Write big-endian data
    buffer[0] = 0x12; buffer[1] = 0x34;
    buffer[2] = 0x56; buffer[3] = 0x78;
    
    Reader reader(buffer.data(), buffer.size());
    std::array<uint16_t, 2> dest{};
    
    reader.read_array_be(bytestream::span<uint16_t>(dest.data(), dest.size()));
    
    EXPECT_EQ(dest[0], 0x1234);
    EXPECT_EQ(dest[1], 0x5678);
}

// ============================================================================
// String Operations
// ============================================================================

TEST_F(ReaderTest, ReadString) {
    std::string test_str = "Hello, World!";
    std::memcpy(buffer.data(), test_str.data(), test_str.size());
    
    Reader reader(buffer.data(), buffer.size());
    std::string result = reader.read_string(test_str.size());
    
    EXPECT_EQ(result, test_str);
    EXPECT_EQ(reader.position(), test_str.size());
}

TEST_F(ReaderTest, ReadSizedStringLE) {
    std::string test_str = "Test";
    uint32_t length = static_cast<uint32_t>(test_str.size());
    
    std::memcpy(buffer.data(), &length, sizeof(length));
    std::memcpy(buffer.data() + sizeof(length), test_str.data(), test_str.size());
    
    Reader reader(buffer.data(), buffer.size());
    std::string result = reader.read_sized_string_le();
    
    EXPECT_EQ(result, test_str);
}

TEST_F(ReaderTest, ReadCString) {
    const char* test_str = "Null-terminated";
    std::strcpy(reinterpret_cast<char*>(buffer.data()), test_str);
    
    Reader reader(buffer.data(), buffer.size());
    std::string result = reader.read_cstring();
    
    EXPECT_EQ(result, test_str);
    EXPECT_EQ(reader.position(), std::strlen(test_str) + 1);
}

TEST_F(ReaderTest, ReadCStringNoTerminator) {
    // Fill buffer without null terminator
    std::fill(buffer.begin(), buffer.end(), 0x41);

    Reader reader(buffer.data(), buffer.size());

    EXPECT_THROW({
        auto s = reader.read_cstring();
        (void)s;
    }, UnderflowException);
}

TEST_F(ReaderTest, ViewString) {
    std::string test_str = "View Test";
    std::memcpy(buffer.data(), test_str.data(), test_str.size());
    
    Reader reader(buffer.data(), buffer.size());
    std::string_view view = reader.view_string(test_str.size());
    
    EXPECT_EQ(view, test_str);
    EXPECT_EQ(reader.position(), test_str.size());
}

// ============================================================================
// Subviews
// ============================================================================

TEST_F(ReaderTest, Subview) {
    for (size_t i = 0; i < 100; ++i) {
        buffer[i] = static_cast<uint8_t>(i);
    }
    
    Reader reader(buffer.data(), 100);
    Reader sub = reader.subview(10, 20);
    
    EXPECT_EQ(sub.size(), 20);
    EXPECT_EQ(sub.position(), 0);
    
    uint8_t value = sub.read<uint8_t>();
    EXPECT_EQ(value, 10);
}

TEST_F(ReaderTest, SubviewToEnd) {
    Reader reader(buffer.data(), 100);
    Reader sub = reader.subview(50);
    
    EXPECT_EQ(sub.size(), 50);
}

TEST_F(ReaderTest, SubviewOutOfBounds) {
    Reader reader(buffer.data(), 10);

    EXPECT_THROW({
        auto tmp = reader.subview(11, 5);
        (void)tmp;
    }, std::out_of_range);

    EXPECT_THROW({
        auto tmp = reader.subview(5, 10);
        (void)tmp;
    }, std::out_of_range);
}

// ============================================================================
// Error Conditions
// ============================================================================

TEST_F(ReaderTest, ReadBeyondEnd) {
    Reader reader(buffer.data(), 4);

    // use the result so nodiscard is satisfied
    auto ok = reader.read<uint32_t>();
    (void)ok;

    EXPECT_THROW({
        auto v = reader.read<uint8_t>();
        (void)v;
    }, UnderflowException);
}

TEST_F(ReaderTest, ReadUnderflow) {
    Reader reader(buffer.data(), 2);

    EXPECT_THROW({
        auto v = reader.read<uint32_t>();
        (void)v;
    }, UnderflowException);
}


// ============================================================================
// Writer Tests
// ============================================================================

class WriterTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buffer;
    
    void SetUp() override {
        buffer.resize(1024, 0);
    }
};

TEST_F(WriterTest, Construction) {
    Writer writer(buffer.data(), buffer.size());
    
    EXPECT_EQ(writer.size(), 1024);
    EXPECT_EQ(writer.position(), 0);
    EXPECT_EQ(writer.remaining(), 1024);
}

TEST_F(WriterTest, WriteUint8) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write<uint8_t>(0x42);
    
    EXPECT_EQ(buffer[0], 0x42);
    EXPECT_EQ(writer.position(), 1);
}

TEST_F(WriterTest, WriteUint16LE) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write_le<uint16_t>(0x1234);
    
    EXPECT_EQ(buffer[0], 0x34);
    EXPECT_EQ(buffer[1], 0x12);
    EXPECT_EQ(writer.position(), 2);
}

TEST_F(WriterTest, WriteUint16BE) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write_be<uint16_t>(0x1234);
    
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(writer.position(), 2);
}

TEST_F(WriterTest, WriteUint32LE) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write_le<uint32_t>(0x12345678);
    
    EXPECT_EQ(buffer[0], 0x78);
    EXPECT_EQ(buffer[1], 0x56);
    EXPECT_EQ(buffer[2], 0x34);
    EXPECT_EQ(buffer[3], 0x12);
    EXPECT_EQ(writer.position(), 4);
}

TEST_F(WriterTest, WriteUint32BE) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write_be<uint32_t>(0x12345678);
    
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(buffer[2], 0x56);
    EXPECT_EQ(buffer[3], 0x78);
    EXPECT_EQ(writer.position(), 4);
}

TEST_F(WriterTest, WriteFloat) {
    Writer writer(buffer.data(), buffer.size());
    float test_val = 3.14159f;
    
    writer.write(test_val);
    
    float result;
    std::memcpy(&result, buffer.data(), sizeof(float));
    EXPECT_FLOAT_EQ(result, test_val);
}

TEST_F(WriterTest, WriteDouble) {
    Writer writer(buffer.data(), buffer.size());
    double test_val = 2.718281828459045;
    
    writer.write(test_val);
    
    double result;
    std::memcpy(&result, buffer.data(), sizeof(double));
    EXPECT_DOUBLE_EQ(result, test_val);
}

TEST_F(WriterTest, WriteBytes) {
    Writer writer(buffer.data(), buffer.size());
    std::array<uint8_t, 5> data = {1, 2, 3, 4, 5};
    
    writer.write_bytes(data.data(), data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(buffer[i], data[i]);
    }
    EXPECT_EQ(writer.position(), 5);
}

TEST_F(WriterTest, WriteArray) {
    Writer writer(buffer.data(), buffer.size());
    std::array<uint16_t, 3> data = {{100, 200, 300}};
    
    writer.write_array(bytestream::span<const uint16_t>(data.data(), data.size()));
    
    uint16_t result[3];
    std::memcpy(result, buffer.data(), sizeof(data));
    
    EXPECT_EQ(result[0], 100);
    EXPECT_EQ(result[1], 200);
    EXPECT_EQ(result[2], 300);
}

TEST_F(WriterTest, WriteArrayBE) {
    Writer writer(buffer.data(), buffer.size());
    std::array<uint16_t, 2> data = {{0x1234, 0x5678}};
    
    writer.write_array_be(bytestream::span<const uint16_t>(data.data(), data.size()));
    
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(buffer[2], 0x56);
    EXPECT_EQ(buffer[3], 0x78);
}

TEST_F(WriterTest, WriteString) {
    Writer writer(buffer.data(), buffer.size());
    std::string test_str = "Hello!";
    
    writer.write_string(test_str);
    
    std::string result(reinterpret_cast<char*>(buffer.data()), test_str.size());
    EXPECT_EQ(result, test_str);
    EXPECT_EQ(writer.position(), test_str.size());
}

TEST_F(WriterTest, WriteSizedStringLE) {
    Writer writer(buffer.data(), buffer.size());
    std::string test_str = "Test";
    
    writer.write_sized_string_le(test_str);
    
    uint32_t length;
    std::memcpy(&length, buffer.data(), sizeof(length));
    EXPECT_EQ(length, 4);
    
    std::string result(reinterpret_cast<char*>(buffer.data() + 4), 4);
    EXPECT_EQ(result, test_str);
}

TEST_F(WriterTest, WriteCString) {
    Writer writer(buffer.data(), buffer.size());
    std::string test_str = "CString";
    
    writer.write_cstring(test_str);
    
    EXPECT_EQ(buffer[test_str.size()], 0);
    EXPECT_EQ(writer.position(), test_str.size() + 1);
}

TEST_F(WriterTest, FillBytes) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.fill_bytes(std::byte{0xAA}, 10);
    
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(buffer[i], 0xAA);
    }
    EXPECT_EQ(writer.position(), 10);
}

TEST_F(WriterTest, ZeroFill) {
    std::fill(buffer.begin(), buffer.end(), 0xFF);
    Writer writer(buffer.data(), buffer.size());
    
    writer.zero_fill(10);
    
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(buffer[i], 0);
    }
}

TEST_F(WriterTest, AlignWithFill) {
    Writer writer(buffer.data(), buffer.size());
    
    writer.write<uint8_t>(0x42);
    EXPECT_EQ(writer.position(), 1);
    
    writer.align(4, std::byte{0xFF});
    EXPECT_EQ(writer.position(), 4);
    
    EXPECT_EQ(buffer[0], 0x42);
    EXPECT_EQ(buffer[1], 0xFF);
    EXPECT_EQ(buffer[2], 0xFF);
    EXPECT_EQ(buffer[3], 0xFF);
}

TEST_F(WriterTest, WriteOverflow) {
    Writer writer(buffer.data(), 4);
    
    writer.write<uint32_t>(0x12345678); // OK
    EXPECT_THROW(writer.write<uint8_t>(0), OverflowException);
}

TEST_F(WriterTest, AsReader) {
    Writer writer(buffer.data(), buffer.size());
    writer.write<uint32_t>(0x12345678);
    
    Reader reader = writer.as_reader();
    EXPECT_EQ(reader.size(), writer.size());
    
    reader.rewind();
    uint32_t value = reader.read<uint32_t>();
    EXPECT_EQ(value, 0x12345678);
}

// ============================================================================
// Round-trip Tests
// ============================================================================

TEST(RoundTripTest, WriteAndRead) {
    std::vector<uint8_t> buffer(1024);
    
    Writer writer(buffer.data(), buffer.size());
    writer.write_le<uint16_t>(0x1234);
    writer.write_le<uint32_t>(0x56789ABC);
    writer.write_le<uint64_t>(0x0011223344556677ULL);
    writer.write<float>(3.14159f);
    writer.write<double>(2.718281828);
    writer.write_sized_string_le("Hello, World!");
    
    Reader reader(buffer.data(), buffer.size());
    
    EXPECT_EQ(reader.read_le<uint16_t>(), 0x1234);
    EXPECT_EQ(reader.read_le<uint32_t>(), 0x56789ABC);
    EXPECT_EQ(reader.read_le<uint64_t>(), 0x0011223344556677ULL);
    EXPECT_FLOAT_EQ(reader.read<float>(), 3.14159f);
    EXPECT_DOUBLE_EQ(reader.read<double>(), 2.718281828);
    EXPECT_EQ(reader.read_sized_string_le(), "Hello, World!");
}

TEST(RoundTripTest, BigEndianRoundTrip) {
    std::vector<uint8_t> buffer(1024);
    
    Writer writer(buffer.data(), buffer.size());
    writer.write_be<uint16_t>(0xABCD);
    writer.write_be<uint32_t>(0x12345678);
    writer.write_sized_string_be("Big Endian Test");
    
    Reader reader(buffer.data(), buffer.size());
    
    EXPECT_EQ(reader.read_be<uint16_t>(), 0xABCD);
    EXPECT_EQ(reader.read_be<uint32_t>(), 0x12345678);
    EXPECT_EQ(reader.read_sized_string_be(), "Big Endian Test");
}

// ============================================================================
// Endianness Tests
// ============================================================================

TEST(EndiannessTest, ByteswapUint16) {
    EXPECT_EQ(byteswap<uint16_t>(0x1234), 0x3412);
}

TEST(EndiannessTest, ByteswapUint32) {
    EXPECT_EQ(byteswap<uint32_t>(0x12345678), 0x78563412);
}

TEST(EndiannessTest, ByteswapUint64) {
    EXPECT_EQ(byteswap<uint64_t>(0x0123456789ABCDEFULL), 0xEFCDAB8967452301ULL);
}

TEST(EndiannessTest, ByteswapInt16) {
    EXPECT_EQ(byteswap<int16_t>(0x1234), 0x3412);
}

TEST(EndiannessTest, DetectEndianness) {
    // At least one should be true
    EXPECT_TRUE(is_little_endian() || is_big_endian());
    
    // But not both
    EXPECT_FALSE(is_little_endian() && is_big_endian());
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}