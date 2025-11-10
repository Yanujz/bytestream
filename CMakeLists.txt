cmake_minimum_required(VERSION 3.20)

project(ByteStream
    VERSION 0.1.0
    DESCRIPTION "Binary I/O library for C++17/20"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(BYTESTREAM_BUILD_TESTS "Build unit tests" ON)
option(BYTESTREAM_BUILD_EXAMPLES "Build examples" ON)
option(BYTESTREAM_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BYTESTREAM_INSTALL "Generate install target" ON)
option(BYTESTREAM_ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan)" OFF)
option(BYTESTREAM_ENABLE_COVERAGE "Enable coverage reporting" OFF)

# -----------------------------------------------------------------------------
# Global C++ standard for the library
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Compiler warnings (for our own targets; not exported)
# -----------------------------------------------------------------------------
if(MSVC)
    set(BYTESTREAM_WARNING_FLAGS /W4 /WX)
else()
    set(BYTESTREAM_WARNING_FLAGS
        -Wall -Wextra -Wpedantic -Werror
        -Wconversion -Wsign-conversion
        -Wcast-align -Wcast-qual
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wshadow
        -Wunused
    )
endif()

# -----------------------------------------------------------------------------
# Header-only library
# -----------------------------------------------------------------------------
add_library(bytestream INTERFACE)
add_library(ByteStream::bytestream ALIAS bytestream)

target_include_directories(bytestream INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(bytestream INTERFACE cxx_std_17)

# -----------------------------------------------------------------------------
# Sanitizers / Coverage (optional)
# -----------------------------------------------------------------------------
if(BYTESTREAM_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

if(BYTESTREAM_ENABLE_COVERAGE AND NOT MSVC)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(BYTESTREAM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(BYTESTREAM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# -----------------------------------------------------------------------------
# Benchmarks
# -----------------------------------------------------------------------------
if(BYTESTREAM_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# -----------------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------------
if(BYTESTREAM_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS bytestream
        EXPORT ByteStreamTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ByteStreamTargets
        FILE ByteStreamTargets.cmake
        NAMESPACE ByteStream::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ByteStream
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ByteStreamConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ByteStream
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ByteStream
    )
endif()
