
include(CTest)
set(FIG_COMMIT "513dc046131f5dfdda47ff485efc2db7243f06b1")

FetchContent_Declare(
    test_data
    GIT_REPOSITORY https://github.com/firemodels/fig.git
    GIT_TAG        ${FIG_COMMIT}
)
FetchContent_MakeAvailable(test_data)
set(FIG_DIR ${test_data_SOURCE_DIR})
set(TEST_SMALL_SMV ${FIG_DIR}/smv/Tests/test_model_small/test_model_small.smv)

add_test(NAME "Execute smokediff" COMMAND smokediff)
add_test(NAME "Execute smokeview" COMMAND smokeview -v)
add_test(NAME "Execute smokezip" COMMAND smokezip)

add_test(NAME "Execute background" COMMAND background)
add_test(NAME "Execute background help" COMMAND background -help)
add_test(NAME "Execute background help_all" COMMAND background -help_all)

add_test(NAME "Execute smokeview help" COMMAND smokeview -help)
add_test(NAME "Execute smokeview help_all" COMMAND smokeview -help_all)
add_test(NAME "Execute smokeview sizes" COMMAND smokeview -sizes ${TEST_SMALL_SMV})
add_test(NAME "Execute smokeview info" COMMAND smokeview -info ${TEST_SMALL_SMV})

# parse_simple_slice
add_executable(parse_simple_slice parse_simple_slice.c)
target_link_libraries(parse_simple_slice PRIVATE libsmv)

# parse_simple_bndf
add_executable(parse_simple_bndf parse_simple_bndf.c)
target_link_libraries(parse_simple_bndf PRIVATE libsmv)

# parse_simple_part
add_executable(parse_simple_part parse_simple_part.c)
target_link_libraries(parse_simple_part PRIVATE libsmv)

# parse_simple_pl3d
add_executable(parse_simple_pl3d parse_simple_pl3d.c)
target_link_libraries(parse_simple_pl3d PRIVATE libsmv)

# parse_cad
add_executable(parse_cad parse_cad.c)
target_link_libraries(parse_cad PRIVATE libsmv)

# test_objects
add_executable(test_objects test_objects.c)
target_link_libraries(test_objects PRIVATE libsmv)

# parse_hvac_vals
add_executable(parse_hvac_vals parse_hvac_vals.c)
target_link_libraries(parse_hvac_vals PRIVATE libsmv)

# test_labels
add_executable(test_labels test_labels.c)
target_link_libraries(test_labels PRIVATE libsmv)

# parse_objects
add_executable(parse_objects parse_objects.c)
target_link_libraries(parse_objects PRIVATE libsmv)

# parse_colorbar
add_executable(parse_colorbar parse_colorbar.c)
target_link_libraries(parse_colorbar PRIVATE libsmv)

# mem_test
add_executable(mem_test mem_test.c)
target_link_libraries(mem_test PRIVATE libsmv)

# test_root_dir
add_executable(test_root_dir test_root_dir.c)
target_link_libraries(test_root_dir PRIVATE libsmv)

# test_iter_dir
add_executable(test_iter_dir test_iter_dir.c)
target_link_libraries(test_iter_dir PRIVATE libsmv)

# test_unicode_paths
add_executable(test_unicode_paths unicode_paths.c)
target_link_libraries(test_unicode_paths PRIVATE libsmv)

# sort_surfs
add_executable(sort_surfs sort_surfs.c)
target_link_libraries(sort_surfs PRIVATE libsmv)


# Arguments to this tests are <slice path> <number of frames in slice>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple Slice - Complete"
    COMMAND parse_simple_slice ${FIG_DIR}/smv/Tests/simple_slice.sf 3)
# Simple slice is a finished slice file with the last of the three frames cut in
# half (to simulate a partially written slice file)
add_test(NAME "Simple Slice - Last Frame Half Truncated"
    COMMAND parse_simple_slice ${FIG_DIR}/smv/Tests/truncated_slice.sf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 2 frames, why 2 frames for boundary
# files?
add_test(NAME "Simple BNDF - Complete"
    COMMAND parse_simple_bndf ${FIG_DIR}/smv/Tests/simple_bndf.bf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PRT5 - Complete"
    COMMAND parse_simple_part ${FIG_DIR}/smv/Tests/simple_part.prt5 3)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PL3d - Complete"
    COMMAND parse_simple_pl3d ${FIG_DIR}/smv/Tests/simple_pl3d.q)

add_test(NAME "Parse CAD"
    COMMAND parse_cad ${CMAKE_SOURCE_DIR}/Verification/Visualization/cad_test.ge1)

add_test(NAME "Test Object API"
    COMMAND test_objects)

# This test will still pass as it will simply try to parse as many objects as
# possible.
add_test(NAME "Test Object API (bad objects)"
    COMMAND test_objects ${CMAKE_SOURCE_DIR}/Tests/bad_objects.svo)
# set_property(TEST "Test Object API (bad objects)" PROPERTY WILL_FAIL true)

add_test(NAME "Test Object API (for bundle)"
    COMMAND parse_objects ${CMAKE_SOURCE_DIR}/Build/for_bundle/objects.svo)

add_test(NAME "Parse Colorbar"
    COMMAND parse_colorbar ${CMAKE_SOURCE_DIR}/Build/for_bundle/colorbars/divergent/CET-D01.csv)

add_test(NAME "Simple HVAC Vals - Complete"
    COMMAND parse_hvac_vals ${FIG_DIR}/smv/Tests/simple_hvac.hvac)

add_test(NAME "Test Labels API"
    COMMAND test_labels)

add_test(NAME "Mem Test"
    COMMAND mem_test)

add_test(NAME "Test Root Dir"
    COMMAND test_root_dir)

add_test(NAME "Test Unicode Paths"
    COMMAND test_unicode_paths ${CMAKE_SOURCE_DIR}/Tests/unicode_test_files)

add_test(NAME "Sort Surfs"
    COMMAND sort_surfs)
