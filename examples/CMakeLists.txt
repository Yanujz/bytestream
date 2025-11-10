# apply coverage to example targets
function(bytestream_example_apply_coverage target_name)
    if (BYTESTREAM_ENABLE_COVERAGE)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${target_name} PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
            target_link_options(${target_name} PRIVATE ${BYTESTREAM_COVERAGE_FLAGS})
        endif()
    endif()
endfunction()

# apply sanitizers to example targets
function(bytestream_example_apply_sanitizers target_name)
    if (BYTESTREAM_ENABLE_SANITIZERS)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${target_name} PRIVATE ${BYTESTREAM_SANITIZER_FLAGS})
            # link step: just the sanitizers
            target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
        endif()
    endif()
endfunction()

add_subdirectory(basic_usage)
add_subdirectory(libversion)
