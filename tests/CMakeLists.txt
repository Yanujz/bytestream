cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(bytestream_tests
    test_reader.cpp
)

set_property(TARGET bytestream_tests PROPERTY CXX_STANDARD 20)
set_property(TARGET bytestream_tests PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET bytestream_tests PROPERTY CXX_EXTENSIONS OFF)

target_link_libraries(bytestream_tests
    PRIVATE
        ByteStream::bytestream
        GTest::gtest
        GTest::gtest_main
)

if(DEFINED BYTESTREAM_WARNING_FLAGS)
    target_compile_options(bytestream_tests PRIVATE ${BYTESTREAM_WARNING_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(bytestream_tests)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS bytestream_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
