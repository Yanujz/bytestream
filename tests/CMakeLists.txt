cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# ------------------------------------------------------------------------------
# Fetch GoogleTest
# ------------------------------------------------------------------------------
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# ------------------------------------------------------------------------------
# collect test sources from subdirs
# ------------------------------------------------------------------------------
set(BYTESTREAM_TEST_MAIN ${CMAKE_CURRENT_LIST_DIR}/main.test.cc)

# init global list
set_property(GLOBAL PROPERTY BYTESTREAM_TEST_SOURCES "")

# helper for subdirs
function(bytestream_add_test_source src)
    get_filename_component(abs_src "${src}" ABSOLUTE)
    set_property(GLOBAL APPEND PROPERTY BYTESTREAM_TEST_SOURCES "${abs_src}")
endfunction()

# ------------------------------------------------------------------------------
# add subdirectories that define tests
# ------------------------------------------------------------------------------
add_subdirectory(reader)
add_subdirectory(writer)
add_subdirectory(roundtrip)
add_subdirectory(endian)

# now retrieve everything the subdirs appended
get_property(BYTESTREAM_TEST_SOURCES GLOBAL PROPERTY BYTESTREAM_TEST_SOURCES)

# ------------------------------------------------------------------------------
# big test executable
# ------------------------------------------------------------------------------
add_executable(bytestream_tests
    ${BYTESTREAM_TEST_MAIN}
    ${BYTESTREAM_TEST_SOURCES}
)

target_link_libraries(bytestream_tests
    PRIVATE
        ByteStream::bytestream
        GTest::gtest
)

target_include_directories(bytestream_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
)

# MSVC: silence strcpy etc. for the main test exe
if (MSVC)
    target_compile_definitions(bytestream_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# propagate coverage from top-level
if (BYTESTREAM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(bytestream_tests PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
    target_link_options(bytestream_tests  PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
endif()

# propagate sanitizers from top-level
if (BYTESTREAM_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(bytestream_tests PRIVATE ${BYTESTREAM_SANITIZER_FLAGS})
    target_link_options(bytestream_tests  PRIVATE ${BYTESTREAM_SANITIZER_FLAGS})
endif()

# reuse warnings if set
if (DEFINED BYTESTREAM_WARNING_FLAGS)
    target_compile_options(bytestream_tests PRIVATE ${BYTESTREAM_WARNING_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(bytestream_tests
    DISCOVERY_TIMEOUT 30
)

# ------------------------------------------------------------------------------
# optional: per-file test executables
# ------------------------------------------------------------------------------
foreach(test_src ${BYTESTREAM_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    set(test_target ${test_name}_test)

    add_executable(${test_target}
        ${BYTESTREAM_TEST_MAIN}
        ${test_src}
    )

    target_link_libraries(${test_target}
        PRIVATE
            ByteStream::bytestream
            GTest::gtest
    )

    target_include_directories(${test_target}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_BINARY_DIR}/include
    )

    if (MSVC)
        target_compile_definitions(${test_target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    if (BYTESTREAM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${test_target} PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
        target_link_options(${test_target}  PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
    endif()

    if (BYTESTREAM_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${test_target} PRIVATE ${BYTESTREAM_SANITIZER_FLAGS})
        target_link_options(${test_target}  PRIVATE ${BYTESTREAM_SANITIZER_FLAGS})
    endif()

    if (DEFINED BYTESTREAM_WARNING_FLAGS)
        target_compile_options(${test_target} PRIVATE ${BYTESTREAM_WARNING_FLAGS})
    endif()

    add_test(NAME ${test_target} COMMAND ${test_target})
endforeach()
