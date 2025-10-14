# env2mod
add_executable(env2mod main.c env2mod.c)
target_include_directories(env2mod PRIVATE .)
target_link_libraries(env2mod PRIVATE smv)
set_source_files_properties(env2mod.c PROPERTIES SKIP_LINTING ON)
