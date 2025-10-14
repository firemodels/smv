
# convert
add_executable(convert main.c)
target_include_directories(convert PRIVATE .)
target_link_libraries(convert PRIVATE smv)
