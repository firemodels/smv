# background
add_executable(background main.c)
target_link_libraries(background PRIVATE smv)
install(TARGETS background)
