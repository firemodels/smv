# pnginfo
add_executable(pnginfo main.c)
target_include_directories(pnginfo PRIVATE .)
target_link_libraries(pnginfo PRIVATE smv)
install(TARGETS pnginfo)
