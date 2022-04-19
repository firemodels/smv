
include(CTest)
add_test(NAME "Execute smokediff" COMMAND smokediff)
add_test(NAME "Execute smokeview" COMMAND smokeview -v)
add_test(NAME "Execute smokezip" COMMAND smokezip)

# parse_simple_slice
add_executable(parse_simple_slice parse_simple_slice.c
    ../Source/smokeview/getdata.c
    ../Source/shared/assert.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_slice PRIVATE
    ../Tests
    ../Source/shared
    ../Source/pthreads
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_slice m)
endif()

add_test(NAME "parse_simple_slice" COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/simple_slice.sf 3)
add_test(NAME "parse_truncated_slice" COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/truncated_slice.sf 2)
