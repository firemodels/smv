# flush
add_executable(flush main.c)
target_include_directories(flush PRIVATE .)
target_link_libraries(flush PRIVATE smv)
install(TARGETS flush)
