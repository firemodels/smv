# background
add_executable(background main.c)
target_link_libraries(background PRIVATE libsmv)
install(TARGETS background)
