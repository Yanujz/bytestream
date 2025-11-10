cmake_minimum_required(VERSION 3.14)

project(ByteStream
    VERSION 0.1.0
    DESCRIPTION "Binary I/O library for C++17/20"
    LANGUAGES CXX
)

# always nice to have
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(BYTESTREAM_BUILD_TESTS       "Build bytestream tests" ON)
option(BYTESTREAM_BUILD_EXAMPLES    "Build bytestream examples" ON)
option(BYTESTREAM_BUILD_BENCHMARKS  "Build bytestream benchmarks" OFF)
option(BYTESTREAM_INSTALL           "Enable install rules for bytestream" ON)

option(BYTESTREAM_ENABLE_SANITIZERS "Enable address/UB sanitizers (Clang/GCC)" ON)
option(BYTESTREAM_ENABLE_COVERAGE   "Enable coverage flags (Clang/GCC)" ON)

# ------------------------------------------------------------------------------
# C++ standard
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Header-only library
# ------------------------------------------------------------------------------
add_library(bytestream INTERFACE)
add_library(ByteStream::bytestream ALIAS bytestream)

target_include_directories(bytestream
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(bytestream INTERFACE cxx_std_17)

# ------------------------------------------------------------------------------
# Warnings (for our own targets, we can export the list)
# ------------------------------------------------------------------------------
if (MSVC)
    set(BYTESTREAM_WARNING_FLAGS /W4 /permissive-)
else()
    set(BYTESTREAM_WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------------------------
# Sanitizers / Coverage: we just prepare variables for subdirs
# ------------------------------------------------------------------------------
set(BYTESTREAM_SANITIZER_FLAGS "")
if (BYTESTREAM_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(BYTESTREAM_SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

set(BYTESTREAM_COVERAGE_FLAGS "")
if (BYTESTREAM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(BYTESTREAM_COVERAGE_FLAGS --coverage -O0 -g)
endif()

# ------------------------------------------------------------------------------
# Examples
# ------------------------------------------------------------------------------
if (BYTESTREAM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------
if (BYTESTREAM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Benchmarks
# ------------------------------------------------------------------------------
if (BYTESTREAM_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------
if (BYTESTREAM_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS bytestream
        EXPORT ByteStreamTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT ByteStreamTargets
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

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/ByteStreamConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ByteStream
    )
endif()
