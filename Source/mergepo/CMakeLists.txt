# mergepo
add_executable(mergepo main.c)
target_include_directories(mergepo PRIVATE .)
target_link_libraries(mergepo PRIVATE smv)
