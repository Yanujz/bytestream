#include <bytestream/core.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace bytestream;

void print_hex(const std::vector<uint8_t> &data, size_t count)
{
    std::cout << "Buffer contents: ";
    for(size_t i = 0; i < std::min(count, data.size()); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << "\n";
}

void example_basic_read_write()
{
    std::cout << "\n=== Basic Read/Write Example ===\n";

    // Create a buffer
    std::vector<uint8_t> buffer(256);

    // Write data
    {
        Writer writer(buffer.data(), buffer.size());

        writer.write<uint8_t>(0x42);
        writer.write_le<uint16_t>(0x1234);
        writer.write_le<uint32_t>(0xDEADBEEF);
        writer.write<float>(3.14159f);

        std::cout << "Written " << writer.position() << " bytes\n";
    }

    print_hex(buffer, 15);

    // Read data back
    {
        Reader reader(buffer.data(), buffer.size());

        uint8_t  val1 = reader.read<uint8_t>();
        uint16_t val2 = reader.read_le<uint16_t>();
        uint32_t val3 = reader.read_le<uint32_t>();
        float    val4 = reader.read<float>();

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

    std::vector<uint8_t> buffer(256);

    // Write strings
    {
        Writer writer(buffer.data(), buffer.size());

        writer.write_sized_string_le("Hello, World!");
        writer.write_cstring("Null-terminated string");

        std::cout << "Written " << writer.position() << " bytes\n";
    }

    // Read strings back
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

    std::vector<uint8_t> buffer(256);

    // Write array
    {
        Writer writer(buffer.data(), buffer.size());

        std::array<int32_t, 5> data = { 10, 20, 30, 40, 50 };
        writer.write_array_le(bytestream::span<const int32_t>(data.data(), data.size()));

        std::cout << "Written array of " << data.size() << " int32_t values\n";
    }

    // Read array back
    {
        Reader reader(buffer.data(), buffer.size());

        std::array<int32_t, 5> data{};
        reader.read_array_le(bytestream::span<int32_t>(data.data(), data.size()));

        std::cout << "Read values: ";
        for (const auto &val : data)
        {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}


void example_endianness()
{
    std::cout << "\n=== Endianness Example ===\n";

    std::vector<uint8_t> buffer(32);

    // Write same value in different endiannesses
    {
        Writer writer(buffer.data(), buffer.size());

        uint32_t value = 0x12345678;

        writer.write_le<uint32_t>(value);  // Little-endian
        writer.write_be<uint32_t>(value);  // Big-endian
        writer.write<uint32_t>(value);     // Native endian

        std::cout << "Original value: 0x" << std::hex << value << std::dec << "\n";
    }

    print_hex(buffer, 12);

    std::cout << "Bytes 0-3: Little-endian (78 56 34 12)\n";
    std::cout << "Bytes 4-7: Big-endian (12 34 56 78)\n";
    std::cout << "Bytes 8-11: Native endian\n";
}

void example_alignment()
{
    std::cout << "\n=== Alignment Example ===\n";

    std::vector<uint8_t> buffer(64);

    Writer writer(buffer.data(), buffer.size());

    // Write a byte
    writer.write<uint8_t>(0xAA);
    std::cout << "Position after uint8: " << writer.position() << "\n";

    // Align to 4-byte boundary
    writer.align(4, std::byte{ 0x00 });
    std::cout << "Position after align(4): " << writer.position() << "\n";

    // Write a uint32
    writer.write<uint32_t>(0xBBBBBBBB);
    std::cout << "Position after uint32: " << writer.position() << "\n";

    // Align to 16-byte boundary
    writer.align(16, std::byte{ 0xFF });
    std::cout << "Position after align(16): " << writer.position() << "\n";

    print_hex(buffer, 20);
}

void example_peek_and_seek()
{
    std::cout << "\n=== Peek and Seek Example ===\n";

    std::vector<uint8_t> buffer = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    Reader reader(buffer.data(), buffer.size());

    // Peek doesn't advance position
    uint8_t val1 = reader.peek<uint8_t>();
    std::cout << "Peeked: 0x" << std::hex << static_cast<int>(val1) << std::dec;
    std::cout << ", Position: " << reader.position() << "\n";

    // Read advances position
    uint8_t val2 = reader.read<uint8_t>();
    std::cout << "Read: 0x" << std::hex << static_cast<int>(val2) << std::dec;
    std::cout << ", Position: " << reader.position() << "\n";

    // Seek to specific position
    reader.seek(3);
    std::cout << "After seek(3), Position: " << reader.position() << "\n";

    uint8_t val3 = reader.read<uint8_t>();
    std::cout << "Read: 0x" << std::hex << static_cast<int>(val3) << std::dec << "\n";

    // Rewind to beginning
    reader.rewind();
    std::cout << "After rewind, Position: " << reader.position() << "\n";
}

void example_subviews()
{
    std::cout << "\n=== Subview Example ===\n";

    std::vector<uint8_t> buffer(100);

    // Fill buffer with sequential values
    for(size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] = static_cast<uint8_t>(i);
    }

    Reader reader(buffer.data(), buffer.size());

    // Create a subview from position 10, length 20
    Reader sub = reader.subview(10, 20);

    std::cout << "Main buffer size: " << reader.size() << "\n";
    std::cout << "Subview size: " << sub.size() << "\n";
    std::cout << "Subview position: " << sub.position() << "\n";

    // Read from subview
    uint8_t val = sub.read<uint8_t>();
    std::cout << "First byte in subview: " << static_cast<int>(val) << " (should be 10)\n";
}

void example_error_handling()
{
    std::cout << "\n=== Error Handling Example ===\n";

    std::vector<uint8_t> buffer(4);

    try {
        Reader reader(buffer.data(), buffer.size());

        // This is OK
        uint32_t val = reader.read_le<uint32_t>();
        (void)val; // silence unused
        std::cout << "Successfully read uint32\n";

        // This will throw
        auto bad = reader.read_le<uint8_t>();
        (void)bad;
    } catch(const UnderflowException &e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }

    try {
        Writer writer(buffer.data(), buffer.size());
        writer.seek(100);  // Out of bounds
    } catch(const std::out_of_range &e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}


void example_binary_protocol()
{
    std::cout << "\n=== Binary Protocol Example ===\n";

    // Simulate a simple packet format:
    // [Header: 4 bytes magic][Version: 2 bytes][Length: 4 bytes][Data: variable][Checksum: 4 bytes]

    std::vector<uint8_t> buffer(256);

    // Encode packet
    {
        Writer writer(buffer.data(), buffer.size());

        const uint32_t    MAGIC   = 0xDEADBEEF;
        const uint16_t    VERSION = 0x0100;
        const std::string payload = "Important data";

        writer.write_be<uint32_t>(MAGIC);
        writer.write_be<uint16_t>(VERSION);
        writer.write_be<uint32_t>(static_cast<uint32_t>(payload.size()));
        writer.write_string(payload);

        // Simple checksum (sum of all bytes)
        uint32_t checksum = 0;
        for(size_t i = 0; i < writer.position(); ++i)
        {
            checksum += buffer[i];
        }
        writer.write_be<uint32_t>(checksum);

        std::cout << "Encoded packet: " << writer.position() << " bytes\n";
    }

    // Decode packet
    {
        Reader reader(buffer.data(), buffer.size());

        uint32_t    magic    = reader.read_be<uint32_t>();
        uint16_t    version  = reader.read_be<uint16_t>();
        uint32_t    length   = reader.read_be<uint32_t>();
        std::string payload  = reader.read_string(length);
        uint32_t    checksum = reader.read_be<uint32_t>();

        std::cout << "Decoded packet:\n";
        std::cout << "  Magic:    0x" << std::hex << magic << std::dec << "\n";
        std::cout << "  Version:  " << (version >> 8) << "." << (version & 0xFF) << "\n";
        std::cout << "  Length:   " << length << "\n";
        std::cout << "  Payload:  \"" << payload << "\"\n";
        std::cout << "  Checksum: 0x" << std::hex << checksum << std::dec << "\n";
    }
}

int main()
{
    std::cout << "ByteStream Library Examples\n";
    std::cout << "===========================\n";

    std::cout << "\nSystem endianness: "
              << (is_little_endian() ? "Little Endian" : "Big Endian") << "\n";

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
