#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <vector>
#include <string>

using namespace bytestream;

TEST(RoundTripTest, WriteAndRead)
{
    std::vector<std::uint8_t> buffer(1024);

    Writer writer(buffer.data(), buffer.size());

    writer.write_le<std::uint16_t>(0x1234);
    writer.write_le<std::uint32_t>(0x56789ABC);
    writer.write_le<std::uint64_t>(0x0011223344556677ULL);
    writer.write<float>(3.14159f);
    writer.write<double>(2.718281828);
    writer.write_sized_string_le("Hello, World!");

    Reader reader(buffer.data(), buffer.size());
    EXPECT_EQ(reader.read_le<std::uint16_t>(), 0x1234);
    EXPECT_EQ(reader.read_le<std::uint32_t>(), 0x56789ABC);
    EXPECT_EQ(reader.read_le<std::uint64_t>(), 0x0011223344556677ULL);
    EXPECT_FLOAT_EQ(reader.read<float>(), 3.14159f);
    EXPECT_DOUBLE_EQ(reader.read<double>(), 2.718281828);
    EXPECT_EQ(reader.read_sized_string_le(), "Hello, World!");
}

TEST(RoundTripTest, BigEndianRoundTrip)
{
    std::vector<std::uint8_t> buffer(1024);

    Writer writer(buffer.data(), buffer.size());

    writer.write_be<std::uint16_t>(0xABCD);
    writer.write_be<std::uint32_t>(0x12345678);
    writer.write_sized_string_be("Big Endian Test");

    Reader reader(buffer.data(), buffer.size());
    EXPECT_EQ(reader.read_be<std::uint16_t>(), 0xABCD);
    EXPECT_EQ(reader.read_be<std::uint32_t>(), 0x12345678);
    EXPECT_EQ(reader.read_sized_string_be(), "Big Endian Test");
}
