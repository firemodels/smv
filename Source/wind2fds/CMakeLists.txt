# wind2fds
add_executable(wind2fds main.c)
target_link_libraries(wind2fds PRIVATE smv)
target_include_directories(wind2fds PRIVATE .)
install(TARGETS wind2fds)
