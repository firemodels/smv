# sh2bat
add_executable(sh2bat sh2bat.c)
target_include_directories(env2mod PRIVATE .)
target_link_libraries(env2mod PRIVATE smv)
install(TARGETS sh2bat)
