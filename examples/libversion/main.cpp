#include <bytestream/version.hpp>
#include <iostream>

int main() {
    // 1) simplest: print version string
    std::cout << "bytestream version: " << bytestream::version_string << "\n";

    // 2) print the complete name
    std::cout << "bytestream name:    " << bytestream::complete_name << "\n";

    // 3) raw components
    std::cout << "major: " << bytestream::version_major
              << ", minor: " << bytestream::version_minor
              << ", patch: " << bytestream::version_patch << "\n";

    // 4) hex form
    std::cout << "hex version: 0x" << std::hex << bytestream::version_hex << std::dec << "\n";

    // 5) aggregate info
    auto v = bytestream::info();
    std::cout << "\n--- via bytestream::info() ---\n";
    std::cout << "string:        " << v.string << "\n";
    std::cout << "lib name:      " << v.lib_name << "\n";
    std::cout << "complete:      " << v.complete << "\n";
    std::cout << "numeric:       " << v.major << "." << v.minor << "." << v.patch << "\n";
    std::cout << "hex:           0x" << std::hex << v.hex << std::dec << "\n";

    if (v.has_suffix) {
        std::cout << "suffix:        " << v.suffix << "\n";
    }
    if (v.has_git) {
        std::cout << "git hash:      " << v.git << "\n";
    }

    // 6) C-style accessors
    std::cout << "\n--- C-style accessors ---\n";
    std::cout << "major(): " << bytestream_version_major()
              << ", minor(): " << bytestream_version_minor()
              << ", patch(): " << bytestream_version_patch() << "\n";
    std::cout << "hex():   0x" << std::hex << bytestream_version_hex() << std::dec << "\n";

    return 0;
}
