# makepo
add_executable(makepo main.c)
target_include_directories(makepo PRIVATE .)
target_link_libraries(makepo PRIVATE smv)
