#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <vector>
#include <array>
#include <cstring>
#include <algorithm>
#include <string>
#include <string_view>

using namespace bytestream;

class ReaderTest : public ::testing::Test {
protected:
    std::vector<std::uint8_t> buffer;

    void SetUp() override
    {
        buffer.resize(1024, 0);
    }
};

TEST_F(ReaderTest, Construction)
{
    Reader reader(buffer.data(), buffer.size());

    EXPECT_EQ(reader.size(), 1024);
    EXPECT_EQ(reader.position(), 0);
    EXPECT_EQ(reader.remaining(), 1024);
    EXPECT_FALSE(reader.empty());
    EXPECT_FALSE(reader.exhausted());
}

TEST_F(ReaderTest, EmptyBuffer)
{
    Reader reader(nullptr, 0);

    EXPECT_EQ(reader.size(), 0);
    EXPECT_TRUE(reader.empty());
    EXPECT_TRUE(reader.exhausted());
}

TEST_F(ReaderTest, SpanConstruction)
{
    bytestream::span<std::uint8_t> span(buffer.data(), buffer.size());
    Reader reader(span);

    EXPECT_EQ(reader.size(), buffer.size());
}

TEST_F(ReaderTest, Seek)
{
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

TEST_F(ReaderTest, SeekOutOfBounds)
{
    Reader reader(buffer.data(), buffer.size());

    EXPECT_THROW(reader.seek(1025), std::out_of_range);
}

TEST_F(ReaderTest, Rewind)
{
    Reader reader(buffer.data(), buffer.size());

    reader.seek(500);
    reader.rewind();
    EXPECT_EQ(reader.position(), 0);
}

TEST_F(ReaderTest, Skip)
{
    Reader reader(buffer.data(), buffer.size());

    reader.skip(10);
    EXPECT_EQ(reader.position(), 10);
    reader.skip(100);
    EXPECT_EQ(reader.position(), 110);
}

TEST_F(ReaderTest, SkipBeyondEnd)
{
    Reader reader(buffer.data(), 10);

    EXPECT_THROW(reader.skip(11), UnderflowException);
}

TEST_F(ReaderTest, Alignment)
{
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

TEST_F(ReaderTest, ReadUint8)
{
    buffer[0] = 0x42;
    Reader       reader(buffer.data(), buffer.size());
    std::uint8_t value = reader.read<std::uint8_t>();
    EXPECT_EQ(value, 0x42);
    EXPECT_EQ(reader.position(), 1);
}

TEST_F(ReaderTest, ReadUint16LE)
{
    buffer[0] = 0x34;
    buffer[1] = 0x12;
    Reader        reader(buffer.data(), buffer.size());
    std::uint16_t value = reader.read_le<std::uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 2);
}

TEST_F(ReaderTest, ReadUint16BE)
{
    buffer[0] = 0x12;
    buffer[1] = 0x34;
    Reader        reader(buffer.data(), buffer.size());
    std::uint16_t value = reader.read_be<std::uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 2);
}

TEST_F(ReaderTest, ReadUint32LE)
{
    buffer[0] = 0x78;
    buffer[1] = 0x56;
    buffer[2] = 0x34;
    buffer[3] = 0x12;
    Reader        reader(buffer.data(), buffer.size());
    std::uint32_t value = reader.read_le<std::uint32_t>();
    EXPECT_EQ(value, 0x12345678);
    EXPECT_EQ(reader.position(), 4);
}

TEST_F(ReaderTest, ReadUint32BE)
{
    buffer[0] = 0x12;
    buffer[1] = 0x34;
    buffer[2] = 0x56;
    buffer[3] = 0x78;
    Reader        reader(buffer.data(), buffer.size());
    std::uint32_t value = reader.read_be<std::uint32_t>();
    EXPECT_EQ(value, 0x12345678);
    EXPECT_EQ(reader.position(), 4);
}

TEST_F(ReaderTest, ReadUint64LE)
{
    for(std::size_t i = 0; i < 8; ++i)
    {
        buffer[i] = static_cast<std::uint8_t>(0x88 - i * 0x11);
    }
    Reader        reader(buffer.data(), buffer.size());
    std::uint64_t value = reader.read_le<std::uint64_t>();
    EXPECT_EQ(value, 0x1122334455667788ULL);
    EXPECT_EQ(reader.position(), 8);
}

TEST_F(ReaderTest, ReadInt32)
{
    std::int32_t test_val = -12345;

    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    Reader       reader(buffer.data(), buffer.size());
    std::int32_t value = reader.read<std::int32_t>();
    EXPECT_EQ(value, test_val);
}

TEST_F(ReaderTest, ReadFloat)
{
    float test_val = 3.14159f;

    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    Reader reader(buffer.data(), buffer.size());
    float  value = reader.read<float>();
    EXPECT_FLOAT_EQ(value, test_val);
}

TEST_F(ReaderTest, ReadDouble)
{
    double test_val = 2.718281828459045;

    std::memcpy(buffer.data(), &test_val, sizeof(test_val));
    Reader reader(buffer.data(), buffer.size());
    double value = reader.read<double>();
    EXPECT_DOUBLE_EQ(value, test_val);
}

TEST_F(ReaderTest, Peek)
{
    buffer[0] = 0x42;
    buffer[1] = 0x43;
    Reader reader(buffer.data(), buffer.size());

    std::uint8_t v1 = reader.peek<std::uint8_t>();
    EXPECT_EQ(v1, 0x42);
    EXPECT_EQ(reader.position(), 0);

    std::uint8_t v2 = reader.peek<std::uint8_t>();
    EXPECT_EQ(v2, 0x42);
    EXPECT_EQ(reader.position(), 0);

    std::uint8_t v3 = reader.read<std::uint8_t>();
    EXPECT_EQ(v3, 0x42);
    EXPECT_EQ(reader.position(), 1);
}

TEST_F(ReaderTest, PeekLE)
{
    buffer[0] = 0x34;
    buffer[1] = 0x12;
    Reader        reader(buffer.data(), buffer.size());
    std::uint16_t value = reader.peek_le<std::uint16_t>();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(reader.position(), 0);
}

TEST_F(ReaderTest, ReadBytes)
{
    for(std::size_t i = 0; i < 10; ++i)
    {
        buffer[i] = static_cast<std::uint8_t>(i);
    }
    Reader reader(buffer.data(), buffer.size());
    std::array<std::uint8_t, 10> dest{};
    reader.read_bytes(dest.data(), 10);
    for(std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(dest[i], i);
    }
    EXPECT_EQ(reader.position(), 10);
}

TEST_F(ReaderTest, ReadBytesSpan)
{
    for(std::size_t i = 0; i < 10; ++i)
    {
        buffer[i] = static_cast<std::uint8_t>(i);
    }
    Reader reader(buffer.data(), buffer.size());
    std::array<std::byte, 10> dest{};
    reader.read_bytes(bytestream::span<std::byte>(dest.data(), dest.size()));
    for(std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(static_cast<std::uint8_t>(dest[i]), i);
    }
}

TEST_F(ReaderTest, ReadArray)
{
    std::array<std::uint16_t, 5> test_data = { { 1, 2, 3, 4, 5 } };

    std::memcpy(buffer.data(), test_data.data(), sizeof(test_data));
    Reader reader(buffer.data(), buffer.size());
    std::array<std::uint16_t, 5> dest{};
    reader.read_array(bytestream::span<std::uint16_t>(dest.data(), dest.size()));
    EXPECT_EQ(dest, test_data);
    EXPECT_EQ(reader.position(), 10);
}

TEST_F(ReaderTest, ReadArrayLE)
{
    buffer[0] = 0x12; buffer[1] = 0x34;
    buffer[2] = 0x56; buffer[3] = 0x78;
    Reader reader(buffer.data(), buffer.size());
    std::array<std::uint16_t, 2> dest{};
    reader.read_array_be(bytestream::span<std::uint16_t>(dest.data(), dest.size()));
    EXPECT_EQ(dest[0], 0x1234);
    EXPECT_EQ(dest[1], 0x5678);
}

TEST_F(ReaderTest, ReadString)
{
    std::string s = "Hello, World!";

    std::memcpy(buffer.data(), s.data(), s.size());
    Reader      reader(buffer.data(), buffer.size());
    std::string res = reader.read_string(s.size());
    EXPECT_EQ(res, s);
    EXPECT_EQ(reader.position(), s.size());
}

TEST_F(ReaderTest, ReadSizedStringLE)
{
    std::string   s   = "Test";
    std::uint32_t len = static_cast<std::uint32_t>(s.size());

    std::memcpy(buffer.data(), &len, sizeof(len));
    std::memcpy(buffer.data() + sizeof(len), s.data(), s.size());
    Reader      reader(buffer.data(), buffer.size());
    std::string res = reader.read_sized_string_le();
    EXPECT_EQ(res, s);
}

TEST_F(ReaderTest, ReadCString)
{
    const char* s = "Null-terminated";

    std::strcpy(reinterpret_cast<char*>(buffer.data()), s);
    Reader      reader(buffer.data(), buffer.size());
    std::string res = reader.read_cstring();
    EXPECT_EQ(res, s);
    EXPECT_EQ(reader.position(), std::strlen(s) + 1);
}

TEST_F(ReaderTest, ReadCStringNoTerminator)
{
    std::fill(buffer.begin(), buffer.end(), 0x41);
    Reader reader(buffer.data(), buffer.size());
    EXPECT_THROW({
        auto tmp = reader.read_cstring();
        (void)tmp;
    }, UnderflowException);
}

TEST_F(ReaderTest, ViewString)
{
    std::string s = "View Test";

    std::memcpy(buffer.data(), s.data(), s.size());
    Reader           reader(buffer.data(), buffer.size());
    std::string_view v = reader.view_string(s.size());
    EXPECT_EQ(v, s);
    EXPECT_EQ(reader.position(), s.size());
}

TEST_F(ReaderTest, Subview)
{
    for(std::size_t i = 0; i < 100; ++i)
    {
        buffer[i] = static_cast<std::uint8_t>(i);
    }
    Reader reader(buffer.data(), 100);
    Reader sub = reader.subview(10, 20);
    EXPECT_EQ(sub.size(), 20);
    EXPECT_EQ(sub.position(), 0);
    std::uint8_t v = sub.read<std::uint8_t>();
    EXPECT_EQ(v, 10);
}

TEST_F(ReaderTest, SubviewToEnd)
{
    Reader reader(buffer.data(), 100);
    Reader sub = reader.subview(50);

    EXPECT_EQ(sub.size(), 50);
}

TEST_F(ReaderTest, SubviewOutOfBounds)
{
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

TEST_F(ReaderTest, ReadBeyondEnd)
{
    Reader reader(buffer.data(), 4);
    auto   ok = reader.read<std::uint32_t>();

    (void)ok;
    EXPECT_THROW({
        auto x = reader.read<std::uint8_t>();
        (void)x;
    }, UnderflowException);
}

TEST_F(ReaderTest, ReadUnderflow)
{
    Reader reader(buffer.data(), 2);

    EXPECT_THROW({
        auto x = reader.read<std::uint32_t>();
        (void)x;
    }, UnderflowException);
}
