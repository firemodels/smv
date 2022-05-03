
include(CTest)

file(GLOB TEST_CASES *.smv)

# Run the script for each visulization case
foreach(smv_path ${TEST_CASES})
    cmake_path(GET smv_path STEM case_name)
    cmake_path(REPLACE_EXTENSION smv_path .ssf
        OUTPUT_VARIABLE script_path)
    if (EXISTS script_path)
        add_test(NAME "Run script ${case_name}" COMMAND smokeview ${smv_path} -runscript WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Verification/WUI)
    endif ()
endforeach()
