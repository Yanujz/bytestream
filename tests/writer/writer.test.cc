#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <vector>
#include <array>
#include <cstring>
#include <algorithm>
#include <string>

using namespace bytestream;

class WriterTest : public ::testing::Test {
protected:
    std::vector<std::uint8_t> buffer;

    void SetUp() override
    {
        buffer.resize(1024, 0);
    }
};

TEST_F(WriterTest, Construction)
{
    Writer writer(buffer.data(), buffer.size());

    EXPECT_EQ(writer.size(), 1024);
    EXPECT_EQ(writer.position(), 0);
    EXPECT_EQ(writer.remaining(), 1024);
}

TEST_F(WriterTest, WriteUint8)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write<std::uint8_t>(0x42);
    EXPECT_EQ(buffer[0], 0x42);
    EXPECT_EQ(writer.position(), 1);
}

TEST_F(WriterTest, WriteUint16LE)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write_le<std::uint16_t>(0x1234);
    EXPECT_EQ(buffer[0], 0x34);
    EXPECT_EQ(buffer[1], 0x12);
    EXPECT_EQ(writer.position(), 2);
}

TEST_F(WriterTest, WriteUint16BE)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write_be<std::uint16_t>(0x1234);
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(writer.position(), 2);
}

TEST_F(WriterTest, WriteUint32LE)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write_le<std::uint32_t>(0x12345678);
    EXPECT_EQ(buffer[0], 0x78);
    EXPECT_EQ(buffer[1], 0x56);
    EXPECT_EQ(buffer[2], 0x34);
    EXPECT_EQ(buffer[3], 0x12);
    EXPECT_EQ(writer.position(), 4);
}

TEST_F(WriterTest, WriteUint32BE)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write_be<std::uint32_t>(0x12345678);
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(buffer[2], 0x56);
    EXPECT_EQ(buffer[3], 0x78);
    EXPECT_EQ(writer.position(), 4);
}

TEST_F(WriterTest, WriteFloat)
{
    Writer writer(buffer.data(), buffer.size());
    float  val = 3.14159f;

    writer.write(val);
    float out;
    std::memcpy(&out, buffer.data(), sizeof(float));
    EXPECT_FLOAT_EQ(out, val);
}

TEST_F(WriterTest, WriteDouble)
{
    Writer writer(buffer.data(), buffer.size());
    double val = 2.718281828459045;

    writer.write(val);
    double out;
    std::memcpy(&out, buffer.data(), sizeof(double));
    EXPECT_DOUBLE_EQ(out, val);
}

TEST_F(WriterTest, WriteBytes)
{
    Writer writer(buffer.data(), buffer.size());
    std::array<std::uint8_t, 5> data = { 1, 2, 3, 4, 5 };

    writer.write_bytes(data.data(), data.size());
    for(std::size_t i = 0; i < data.size(); ++i)
    {
        EXPECT_EQ(buffer[i], data[i]);
    }
    EXPECT_EQ(writer.position(), 5);
}

TEST_F(WriterTest, WriteArray)
{
    Writer writer(buffer.data(), buffer.size());
    std::array<std::uint16_t, 3> data = { { 100, 200, 300 } };

    writer.write_array(bytestream::span<const std::uint16_t>(data.data(), data.size()));
    std::uint16_t out[3];
    std::memcpy(out, buffer.data(), sizeof(data));
    EXPECT_EQ(out[0], 100);
    EXPECT_EQ(out[1], 200);
    EXPECT_EQ(out[2], 300);
}

TEST_F(WriterTest, WriteArrayBE)
{
    Writer writer(buffer.data(), buffer.size());
    std::array<std::uint16_t, 2> data = { { 0x1234, 0x5678 } };

    writer.write_array_be(bytestream::span<const std::uint16_t>(data.data(), data.size()));
    EXPECT_EQ(buffer[0], 0x12);
    EXPECT_EQ(buffer[1], 0x34);
    EXPECT_EQ(buffer[2], 0x56);
    EXPECT_EQ(buffer[3], 0x78);
}

TEST_F(WriterTest, WriteString)
{
    Writer      writer(buffer.data(), buffer.size());
    std::string s = "Hello!";

    writer.write_string(s);
    std::string out(reinterpret_cast<char*>(buffer.data()), s.size());
    EXPECT_EQ(out, s);
    EXPECT_EQ(writer.position(), s.size());
}

TEST_F(WriterTest, WriteSizedStringLE)
{
    Writer      writer(buffer.data(), buffer.size());
    std::string s = "Test";

    writer.write_sized_string_le(s);
    std::uint32_t len;
    std::memcpy(&len, buffer.data(), sizeof(len));
    EXPECT_EQ(len, 4);
    std::string out(reinterpret_cast<char*>(buffer.data() + 4), 4);
    EXPECT_EQ(out, s);
}

TEST_F(WriterTest, WriteCString)
{
    Writer      writer(buffer.data(), buffer.size());
    std::string s = "CString";

    writer.write_cstring(s);
    EXPECT_EQ(buffer[s.size()], 0);
    EXPECT_EQ(writer.position(), s.size() + 1);
}

TEST_F(WriterTest, FillBytes)
{
    Writer writer(buffer.data(), buffer.size());

    writer.fill_bytes(std::byte{ 0xAA }, 10);
    for(std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(buffer[i], 0xAA);
    }
    EXPECT_EQ(writer.position(), 10);
}

TEST_F(WriterTest, ZeroFill)
{
    std::fill(buffer.begin(), buffer.end(), 0xFF);
    Writer writer(buffer.data(), buffer.size());
    writer.zero_fill(10);
    for(std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(buffer[i], 0);
    }
}

TEST_F(WriterTest, AlignWithFill)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write<std::uint8_t>(0x42);
    EXPECT_EQ(writer.position(), 1);
    writer.align(4, std::byte{ 0xFF });
    EXPECT_EQ(writer.position(), 4);
    EXPECT_EQ(buffer[0], 0x42);
    EXPECT_EQ(buffer[1], 0xFF);
    EXPECT_EQ(buffer[2], 0xFF);
    EXPECT_EQ(buffer[3], 0xFF);
}

TEST_F(WriterTest, WriteOverflow)
{
    Writer writer(buffer.data(), 4);

    writer.write<std::uint32_t>(0x12345678);
    EXPECT_THROW(writer.write<std::uint8_t>(0), OverflowException);
}

TEST_F(WriterTest, AsReader)
{
    Writer writer(buffer.data(), buffer.size());

    writer.write<std::uint32_t>(0x12345678);
    Reader reader = writer.as_reader();
    EXPECT_EQ(reader.size(), writer.size());
    reader.rewind();
    std::uint32_t v = reader.read<std::uint32_t>();
    EXPECT_EQ(v, 0x12345678);
}
