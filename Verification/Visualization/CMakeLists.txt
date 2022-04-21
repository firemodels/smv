
include(CTest)

file(GLOB TEST_CASES *.smv)

# Run the script for each visulization case
foreach(case ${TEST_CASES})
    cmake_path(GET case STEM case_name)
    add_test(NAME "Run script ${case_name}" COMMAND smokeview ${case} -runscript WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Verification/Visualization)
endforeach()


