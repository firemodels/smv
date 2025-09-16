# pnginfo
add_executable(pnginfo main.c)
target_link_libraries(pnginfo PRIVATE libsmv)
install(TARGETS pnginfo)
