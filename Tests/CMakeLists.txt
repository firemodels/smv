
include(CTest)
add_test(NAME "Execute smokediff" COMMAND smokediff)
add_test(NAME "Execute smokeview" COMMAND smokeview)
add_test(NAME "Execute smokezip" COMMAND smokezip)


# parse_simple_slice
add_executable(parse_simple_slice parse_simple_slice.c
    # ../Source/smokeview/getdata.c
    # ../Source/shared/assert.c ../Source/shared/dmalloc.c ../Source/shared/file_util.c
    # ../Source/shared/string_util.c ../Source/shared/md5.c ../Source/shared/sha1.c ../Source/shared/sha256.c
    # ../Source/shared/stdio_buffer.c
)

target_include_directories(parse_simple_slice PRIVATE
    ../Tests
    ../Source/shared
    # ../Source/smokeview
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_slice m)
endif()

add_test(NAME "parse_simple_slice" COMMAND parse_simple_slice)
