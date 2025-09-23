
# fds2fed
add_executable(fds2fed
    main.c
    fds2fed.c
)
target_include_directories(fds2fed PRIVATE .)
target_link_libraries(fds2fed PRIVATE smv)
