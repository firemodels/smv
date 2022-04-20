
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

# parse_simple_bndf
add_executable(parse_simple_bndf parse_simple_bndf.c
    ../Source/smokeview/getdata.c
    ../Source/shared/assert.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_bndf PRIVATE
    ../Tests
    ../Source/shared
    ../Source/pthreads
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_bndf m)
endif()

# Arguments to this tests are <slice path> <number of frames in slice>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple Slice - Complete"
    COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/simple_slice.sf 3)
# Simple slice is a finished slice file with the last of the three frames cut in
# half (to simulate a partially written slice file)
add_test(NAME "Simple Slice - Last Frame Half Truncated"
    COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/truncated_slice.sf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple BNDF - Complete"
    COMMAND parse_simple_bndf ${CMAKE_SOURCE_DIR}/Tests/simple_bndf.bf 2)
