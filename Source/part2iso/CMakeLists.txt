add_executable(part2iso main.c CNVpart.c readfiles.c)
target_include_directories(part2iso PRIVATE .)
target_link_libraries(part2iso PRIVATE smv)
install(TARGETS part2iso)