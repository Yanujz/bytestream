add_executable(example_libversion
    main.cpp
)

target_link_libraries(example_libversion
    PRIVATE
        ByteStream::bytestream
)

bytestream_example_apply_coverage(example_libversion)
bytestream_example_apply_sanitizers(example_libversion)
