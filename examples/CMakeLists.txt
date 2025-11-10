add_executable(basic_usage
    basic_usage.cpp
)

target_link_libraries(basic_usage
    PRIVATE
        ByteStream::bytestream
)

target_compile_options(basic_usage
    PRIVATE
        ${BYTESTREAM_WARNING_FLAGS}
)