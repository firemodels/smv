# background
add_executable(background main.c)
target_include_directories(background PRIVATE .)
target_link_libraries(background PRIVATE smv)
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(background PRIVATE -Wno-unused-variable)
endif()
install(TARGETS background)
