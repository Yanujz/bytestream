cmake_minimum_required(VERSION 3.14)

project(ByteStream
    VERSION 0.1.0
    DESCRIPTION "Binary I/O library for C++17/20"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BYTESTREAM_BUILD_TESTS      "Build tests" ON)
option(BYTESTREAM_BUILD_EXAMPLES   "Build examples" ON)
option(BYTESTREAM_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(BYTESTREAM_INSTALL          "Install headers/targets" ON)
option(BYTESTREAM_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(BYTESTREAM_ENABLE_COVERAGE   "Enable coverage flags" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# library (header-only)
add_library(bytestream INTERFACE)
add_library(ByteStream::bytestream ALIAS bytestream)

target_include_directories(bytestream
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(bytestream INTERFACE cxx_std_17)

if(MSVC)
    set(BYTESTREAM_WARNING_FLAGS /W4 /permissive-)
else()
    set(BYTESTREAM_WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

if (MSVC)
    target_compile_definitions(bytestream_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# version header generation
set(BYTESTREAM_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(BYTESTREAM_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(BYTESTREAM_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(BYTESTREAM_VERSION_SUFFIX "" CACHE STRING "Optional version suffix")

set(BYTESTREAM_GIT_HASH "")
find_package(Git QUIET)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE BYTESTREAM_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(BYTESTREAM_VERSION_SUFFIX)
    set(BYTESTREAM_HAS_SUFFIX 1)
else()
    set(BYTESTREAM_HAS_SUFFIX 0)
endif()

if(BYTESTREAM_GIT_HASH)
    set(BYTESTREAM_HAS_GIT 1)
else()
    set(BYTESTREAM_HAS_GIT 0)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/bytestream/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/bytestream/version.hpp
    @ONLY
)

target_include_directories(bytestream
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

# flags for subdirs
set(BYTESTREAM_SANITIZER_FLAGS "")
if(BYTESTREAM_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(BYTESTREAM_SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

set(BYTESTREAM_COVERAGE_FLAGS "")
if(BYTESTREAM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(BYTESTREAM_COVERAGE_FLAGS --coverage -O0 -g)
endif()

# tests
if(BYTESTREAM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# examples
if(BYTESTREAM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# benchmarks
if(BYTESTREAM_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# install
if(BYTESTREAM_INSTALL)
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
        FILES ${CMAKE_CURRENT_BINARY_DIR}/include/bytestream/version.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bytestream
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
