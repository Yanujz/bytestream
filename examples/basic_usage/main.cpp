#include <bytestream/core.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <iomanip>   // for std::setw, std::setfill
#include <algorithm> // for std::min

using namespace bytestream;

void print_hex(const std::vector<std::uint8_t> &data, std::size_t count)
{
    std::cout << "Buffer contents: ";
    const std::size_t n = std::min(count, data.size());
    for(std::size_t i = 0; i < n; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << "\n";
}

void example_basic_read_write()
{
    std::cout << "\n=== Basic Read/Write Example ===\n";

    std::vector<std::uint8_t> buffer(256);

    // write
    {
        Writer writer(buffer.data(), buffer.size());

        writer.write<std::uint8_t>(0x42);
        writer.write_le<std::uint16_t>(0x1234);
        writer.write_le<std::uint32_t>(0xDEADBEEF);
        writer.write<float>(3.14159f);

        std::cout << "Written " << writer.position() << " bytes\n";
    }

    print_hex(buffer, 15);

    // read
    {
        Reader reader(buffer.data(), buffer.size());

        std::uint8_t  val1 = reader.read<std::uint8_t>();
        std::uint16_t val2 = reader.read_le<std::uint16_t>();
        std::uint32_t val3 = reader.read_le<std::uint32_t>();
        float         val4 = reader.read<float>();

        std::cout << "Read values:\n";
        std::cout << "  uint8:  0x" << std::hex << static_cast<int>(val1) << "\n";
        std::cout << "  uint16: 0x" << val2 << "\n";
        std::cout << "  uint32: 0x" << val3 << "\n";
        std::cout << std::dec;
        std::cout << "  float:  " << val4 << "\n";
    }
}

void example_strings()
{
    std::cout << "\n=== String Operations Example ===\n";

    std::vector<std::uint8_t> buffer(256);

    // write
    {
        Writer writer(buffer.data(), buffer.size());

        writer.write_sized_string_le("Hello, World!");
        writer.write_cstring("Null-terminated string");

        std::cout << "Written " << writer.position() << " bytes\n";
    }

    // read
    {
        Reader reader(buffer.data(), buffer.size());

        std::string str1 = reader.read_sized_string_le();
        std::string str2 = reader.read_cstring();

        std::cout << "String 1: \"" << str1 << "\"\n";
        std::cout << "String 2: \"" << str2 << "\"\n";
    }
}

void example_arrays()
{
    std::cout << "\n=== Array Operations Example ===\n";

    std::vector<std::uint8_t> buffer(256);

    // write array
    {
        Writer writer(buffer.data(), buffer.size());

        std::array<std::int32_t, 5> data = { 10, 20, 30, 40, 50 };

        writer.write_array(bytestream::span<const std::int32_t>(
            data.data(), data.size()
        ));

        std::cout << "Written array of " << data.size() << " int32_t values\n";
    }

    // read array back
    {
        Reader reader(buffer.data(), buffer.size());

        std::array<std::int32_t, 5> data{};

        reader.read_array_le(bytestream::span<std::int32_t>(
            data.data(), data.size()
        ));

        std::cout << "Read values: ";
        for(const auto &val : data)
        {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}

void example_endianness()
{
    std::cout << "\n=== Endianness Example ===\n";

    std::vector<std::uint8_t> buffer(32);

    {
        Writer writer(buffer.data(), buffer.size());

        std::uint32_t value = 0x12345678;

        writer.write_le<std::uint32_t>(value);  // bytes 0..3
        writer.write_be<std::uint32_t>(value);  // bytes 4..7
        writer.write<std::uint32_t>(value);     // bytes 8..11, native

        std::cout << "Original value: 0x" << std::hex << value << std::dec << "\n";
    }

    print_hex(buffer, 12);

    std::cout << "Bytes 0-3:  little-endian (78 56 34 12)\n";
    std::cout << "Bytes 4-7:  big-endian    (12 34 56 78)\n";
    std::cout << "Bytes 8-11: native-endian (depends on platform)\n";
}

void example_alignment()
{
    std::cout << "\n=== Alignment Example ===\n";

    std::vector<std::uint8_t> buffer(64);
    Writer writer(buffer.data(), buffer.size());

    writer.write<std::uint8_t>(0xAA);
    std::cout << "Position after uint8: " << writer.position() << "\n";

    writer.align(4, std::byte{ 0x00 });
    std::cout << "Position after align(4): " << writer.position() << "\n";

    writer.write<std::uint32_t>(0xBBBBBBBB);
    std::cout << "Position after uint32: " << writer.position() << "\n";

    writer.align(16, std::byte{ 0xFF });
    std::cout << "Position after align(16): " << writer.position() << "\n";

    print_hex(buffer, 20);
}

void example_peek_and_seek()
{
    std::cout << "\n=== Peek and Seek Example ===\n";

    std::vector<std::uint8_t> buffer = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    Reader reader(buffer.data(), buffer.size());

    std::uint8_t v1 = reader.peek<std::uint8_t>();
    std::cout << "Peeked: 0x" << std::hex << static_cast<int>(v1)
              << std::dec << ", Position: " << reader.position() << "\n";

    std::uint8_t v2 = reader.read<std::uint8_t>();
    std::cout << "Read:   0x" << std::hex << static_cast<int>(v2)
              << std::dec << ", Position: " << reader.position() << "\n";

    reader.seek(3);
    std::cout << "After seek(3), Position: " << reader.position() << "\n";

    std::uint8_t v3 = reader.read<std::uint8_t>();
    std::cout << "Read:   0x" << std::hex << static_cast<int>(v3) << std::dec << "\n";

    reader.rewind();
    std::cout << "After rewind, Position: " << reader.position() << "\n";
}

void example_subviews()
{
    std::cout << "\n=== Subview Example ===\n";

    std::vector<std::uint8_t> buffer(100);
    for(std::size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] = static_cast<std::uint8_t>(i);
    }

    Reader reader(buffer.data(), buffer.size());

    Reader sub = reader.subview(10, 20);

    std::cout << "Main buffer size: " << reader.size() << "\n";
    std::cout << "Subview size:     " << sub.size() << "\n";

    std::uint8_t val = sub.read<std::uint8_t>();
    std::cout << "First byte in subview: " << static_cast<int>(val)
              << " (should be 10)\n";
}

void example_error_handling()
{
    std::cout << "\n=== Error Handling Example ===\n";

    std::vector<std::uint8_t> buffer(4);

    try {
        Reader reader(buffer.data(), buffer.size());

        std::uint32_t ok = reader.read_le<std::uint32_t>();
        (void)ok;
        std::cout << "Successfully read uint32\n";

        auto bad = reader.read_le<std::uint8_t>();
        (void)bad;
    }
    catch(const UnderflowException &e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }

    try {
        Writer writer(buffer.data(), buffer.size());
        writer.seek(100);
    }
    catch(const std::out_of_range &e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

void example_binary_protocol()
{
    std::cout << "\n=== Binary Protocol Example ===\n";

    std::vector<std::uint8_t> buffer(256);

    // encode
    {
        Writer writer(buffer.data(), buffer.size());

        const std::uint32_t MAGIC   = 0xDEADBEEF;
        const std::uint16_t VERSION = 0x0100;
        const std::string   payload = "Important data";

        writer.write_be<std::uint32_t>(MAGIC);
        writer.write_be<std::uint16_t>(VERSION);
        writer.write_be<std::uint32_t>(static_cast<std::uint32_t>(payload.size()));
        writer.write_string(payload);

        std::uint32_t checksum = 0;
        for(std::size_t i = 0; i < writer.position(); ++i)
        {
            checksum += buffer[i];
        }
        writer.write_be<std::uint32_t>(checksum);

        std::cout << "Encoded packet: " << writer.position() << " bytes\n";
    }

    // decode
    {
        Reader reader(buffer.data(), buffer.size());

        std::uint32_t magic    = reader.read_be<std::uint32_t>();
        std::uint16_t version  = reader.read_be<std::uint16_t>();
        std::uint32_t length   = reader.read_be<std::uint32_t>();
        std::string   payload  = reader.read_string(length);
        std::uint32_t checksum = reader.read_be<std::uint32_t>();

        std::cout << "Decoded packet:\n";
        std::cout << "  Magic:   0x" << std::hex << magic << std::dec << "\n";
        std::cout << "  Version: " << (version >> 8) << "." << (version & 0xFF) << "\n";
        std::cout << "  Length:  " << length << "\n";
        std::cout << "  Payload: \"" << payload << "\"\n";
        std::cout << "  Checksum: 0x" << std::hex << checksum << std::dec << "\n";
    }
}

int main()
{
    std::cout << "ByteStream Library Examples\n";
    std::cout << "===========================\n";

    std::cout << "\nSystem endianness: "
              << (is_little_endian() ? "Little Endian" : "Big Endian")
              << "\n";

    example_basic_read_write();
    example_strings();
    example_arrays();
    example_endianness();
    example_alignment();
    example_peek_and_seek();
    example_subviews();
    example_error_handling();
    example_binary_protocol();

    std::cout << "\nAll examples completed successfully!\n";
    return 0;
}
