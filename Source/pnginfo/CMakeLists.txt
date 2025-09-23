# pnginfo
add_executable(pnginfo main.c)
target_link_libraries(pnginfo PRIVATE smv)
install(TARGETS pnginfo)
