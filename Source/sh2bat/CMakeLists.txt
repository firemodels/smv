# sh2bat
add_executable(sh2bat sh2bat.c)
target_link_libraries(env2mod PRIVATE smv)
install(TARGETS sh2bat)
