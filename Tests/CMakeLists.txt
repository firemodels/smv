
include(CTest)
add_test(NAME "Execute smokediff" COMMAND smokediff)
add_test(NAME "Execute smokeview" COMMAND smokeview -v)
add_test(NAME "Execute smokezip" COMMAND smokezip)

# parse_simple_slice
add_executable(parse_simple_slice parse_simple_slice.c
    ../Source/shared/file_util.c
    ../Source/shared/getdata.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_slice PRIVATE
    ../Tests
    ../Source/shared
)
if (WIN32)
    target_include_directories(parse_simple_slice PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_slice m)
endif()

# parse_simple_bndf
add_executable(parse_simple_bndf parse_simple_bndf.c
    ../Source/shared/getdata.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_bndf PRIVATE
    ../Tests
    ../Source/shared
)
if (WIN32)
    target_include_directories(parse_simple_bndf PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_bndf m)
endif()


# parse_simple_part
add_executable(parse_simple_part parse_simple_part.c
    ../Source/shared/getdata.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_part PRIVATE
    ../Tests
    ../Source/shared
)
if (WIN32)
    target_include_directories(parse_simple_part PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_part m)
endif()


# parse_simple_pl3d
add_executable(parse_simple_pl3d parse_simple_pl3d.c
    ../Source/shared/getdata.c
    ../Source/shared/dmalloc.c
)

target_include_directories(parse_simple_pl3d PRIVATE
    ../Tests
    ../Source/shared
)
if (WIN32)
    target_include_directories(parse_simple_pl3d PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(parse_simple_pl3d m)
endif()


# test_objects
add_executable(test_objects test_objects.c
    ../Source/shared/dmalloc.c
    ../Source/shared/readobject.c
    ../Source/shared/string_util.c
    ../Source/shared/file_util.c
    ../Source/shared/stdio_buffer.c
    ../Source/shared/sha256.c
    ../Source/shared/sha1.c
    ../Source/shared/md5.c
)

target_include_directories(test_objects PRIVATE
    ../Tests
    ../Source/shared
    ../Source/glew
)
if (WIN32)
    target_include_directories(test_objects PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(test_objects m)
endif()

# Arguments to this tests are <slice path> <number of frames in slice>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple Slice - Complete"
    COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/fig/smv/Tests/simple_slice.sf 3)
# Simple slice is a finished slice file with the last of the three frames cut in
# half (to simulate a partially written slice file)
add_test(NAME "Simple Slice - Last Frame Half Truncated"
    COMMAND parse_simple_slice ${CMAKE_SOURCE_DIR}/Tests/fig/smv/Tests/truncated_slice.sf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 2 frames, why 2 frames for boundary
# files?
add_test(NAME "Simple BNDF - Complete"
    COMMAND parse_simple_bndf ${CMAKE_SOURCE_DIR}/Tests/fig/smv/Tests/simple_bndf.bf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PRT5 - Complete"
COMMAND parse_simple_part ${CMAKE_SOURCE_DIR}/Tests/fig/smv/Tests/simple_part.prt5 3)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PL3d - Complete"
COMMAND parse_simple_pl3d ${CMAKE_SOURCE_DIR}/Tests/fig/smv/Tests/simple_pl3d.q)

add_test(NAME "Test Object API"
COMMAND test_objects ${CMAKE_SOURCE_DIR}/Tests/bad_objects.svo)
