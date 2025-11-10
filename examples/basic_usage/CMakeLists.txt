add_executable(example_basic_usage
    main.cpp
)

target_link_libraries(example_basic_usage
    PRIVATE
        ByteStream::bytestream
)

bytestream_example_apply_coverage(example_basic_usage)
bytestream_example_apply_sanitizers(example_basic_usage)
