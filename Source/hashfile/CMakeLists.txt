# hashfile
add_executable(hashfile main.c)
target_link_libraries(hashfile PRIVATE smv)
install(TARGETS hashfile)
