# timep
add_executable(timep main.c)
target_link_libraries(timep PRIVATE libsmv)
install(TARGETS timep)
