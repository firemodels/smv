# timep
add_executable(timep main.c)
target_link_libraries(timep PRIVATE smv)
target_include_directories(timep PRIVATE .)
install(TARGETS timep)
