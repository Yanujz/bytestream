#include <gtest/gtest.h>
#include <bytestream/core.hpp>
#include <cstdint>

using namespace bytestream;

TEST(EndiannessTest, ByteswapUint16)
{
    EXPECT_EQ(byteswap<std::uint16_t>(0x1234), 0x3412);
}

TEST(EndiannessTest, ByteswapUint32)
{
    EXPECT_EQ(byteswap<std::uint32_t>(0x12345678), 0x78563412);
}

TEST(EndiannessTest, ByteswapUint64)
{
    EXPECT_EQ(byteswap<std::uint64_t>(0x0123456789ABCDEFULL), 0xEFCDAB8967452301ULL);
}

TEST(EndiannessTest, ByteswapInt16)
{
    EXPECT_EQ(byteswap<std::int16_t>(0x1234), 0x3412);
}

TEST(EndiannessTest, DetectEndianness)
{
    EXPECT_TRUE(is_little_endian() || is_big_endian());
    EXPECT_FALSE(is_little_endian() && is_big_endian());
}
