
include(CTest)

file(GLOB TEST_CASES *.smv)

# Run the script for each visulization case
foreach(case ${TEST_CASES})
    add_test(NAME "Run script ${case}" COMMAND smokeview ${case} -runscript)
endforeach()


