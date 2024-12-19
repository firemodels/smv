# hashfile
add_executable(hashfile main.c)
target_link_libraries(hashfile PRIVATE libsmv)
install(TARGETS hashfile)
